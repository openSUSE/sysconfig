/*
 * udevmountd
 *
 * Automount daemon for udev
 *
 * Copyright (c) 2008 Hannes Reinecke <hare@suse.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>

typedef enum {
	MOUNT_UNKNOWN,
	MOUNT_CHECK,
	MOUNT_FSCK,
	MOUNT_REMOVE,
} mount_ops;

typedef enum {
	MOUNT_UNCHECKED,
	MOUNT_SKIPPED,
	MOUNT_CLEAN,
	MOUNT_CORRECTABLE,
	MOUNT_UNCORRECTED,
	MOUNT_FAILED,
	MOUNT_MOUNTED,
} mount_status;

char *mount_status_str[] = {
	"unchecked",
	"skipped",
	"clean",
	"correctable",
	"uncorrected",
	"failed",
	"mounted",
};

#ifdef TEST
#define DEFAULT_LOGLEVEL LOG_DEBUG
#else
#define DEFAULT_LOGLEVEL LOG_DEBUG
#endif

/* How log to wait for the background process to terminate */
#define SIGNAL_TIMEOUT 10

/* Multipath daemon program */
#define MPATHD_PROG "/sbin/multipathd"

/* Fsck program to call */
#ifdef TEST
#define FSCK_PROG "./fsck_test"
#else
#define FSCK_PROG "/sbin/fsck"
#endif

/* Mount program to call */
#define MOUNT_PROG "/bin/mount"

/* Directory for the lock files */
#define LOCK_DIR "/dev/.udev/mountd"

static char *dev;
static char *mnt;
static char *action;
static int major;
static int minor;
static unsigned int passno;
static unsigned int logging = DEFAULT_LOGLEVEL;

static pid_t child_pid;
static int lock_fd;

#define READ_END 0
#define WRITE_END 1

#define err(fmt, args...)			\
	if (logging >= LOG_ERR)			\
		syslog(LOG_ERR, fmt, ##args)

#define warn(fmt, args...)			\
	if (logging >= LOG_WARNING)		\
		syslog(LOG_WARNING, fmt, ##args)

#define notice(fmt, args...)			\
	if (logging >= LOG_NOTICE)		\
		syslog(LOG_NOTICE, fmt, ##args)

#define info(fmt, args...)			\
	if (logging >= LOG_INFO)		\
		syslog(LOG_INFO, fmt, ##args)

#define dbg(fmt, args...)			\
	if (logging >= LOG_DEBUG)		\
		syslog(LOG_DEBUG, fmt, ##args)

/*
 * terminate_child - signal handler for background process
 *
 * The background process starts the external programs
 * as a child process, so we need to terminate that, too.
 * And we should record the status in the lockfile.
 */
static void terminate_child(int signo)
{
	char buf[] = "failed";
	int num;

	if (child_pid > 0) {
		notice("Sending %d\n", signo);
		kill(child_pid, SIGKILL);
	}
	info("Closing lock file\n");
	lseek(lock_fd, 0, SEEK_SET);
	num = write(lock_fd, buf, 20);
	close(lock_fd);
	lock_fd = -1;
	exit(1);
}

/*
 * run_program - Run external program
 *
 * Copied from udev.
 */
static int run_program(char **argv, char *buf, int len)
{
	int status;
	int outpipe[2] = {-1, -1};
	int errpipe[2] = {-1, -1};
	int devnull;
	int retval = 0;
	struct sigaction sa;

	/* prepare pipes from child to parent */
	if (pipe(outpipe) != 0) {
		err("pipe failed: %s\n", strerror(errno));
		return -1;
	}
	if (pipe(errpipe) != 0) {
		err("pipe failed: %s\n", strerror(errno));
		return -1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = terminate_child;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGTERM, &sa, NULL) < 0) {
		err("can't install signal handler: %s\n", strerror(errno));
	}

	child_pid = fork();
	switch(child_pid) {
	case 0:
		/* child closes parent ends of pipes */
		if (outpipe[READ_END] > 0)
			close(outpipe[READ_END]);
		if (errpipe[READ_END] > 0)
			close(errpipe[READ_END]);

		/* discard child output or connect to pipe */
		devnull = open("/dev/null", O_RDWR);
		if (devnull > 0) {
			dup2(devnull, STDIN_FILENO);
			if (outpipe[WRITE_END] < 0)
				dup2(devnull, STDOUT_FILENO);
			if (errpipe[WRITE_END] < 0)
				dup2(devnull, STDERR_FILENO);
			close(devnull);
		} else {
			err("open /dev/null failed: %s\n", strerror(errno));
		}
		if (outpipe[WRITE_END] > 0) {
			dup2(outpipe[WRITE_END], STDOUT_FILENO);
			close(outpipe[WRITE_END]);
		}
		if (errpipe[WRITE_END] > 0) {
			dup2(errpipe[WRITE_END], STDERR_FILENO);
			close(errpipe[WRITE_END]);
		}
		execv(argv[0], argv);
		if (errno == ENOENT || errno == ENOTDIR) {
			/* may be on a filesytem currently not mounted */
			notice("program '%s' not found\n", argv[0]);
		} else {
			/* other problems */
			err("exec of program '%s' failed\n", argv[0]);
		}
		_exit(1);
	case -1:
		err("fork of '%s' failed: %s\n", argv[0], strerror(errno));
		return -1;
	default:
		if (buf)
			buf[0] = '\0';
		/* read from child if requested */
		if (outpipe[READ_END] > 0 || errpipe[READ_END] > 0) {
			ssize_t count;

			/* parent closes child ends of pipes */
			if (outpipe[WRITE_END] > 0)
				close(outpipe[WRITE_END]);
			if (errpipe[WRITE_END] > 0)
				close(errpipe[WRITE_END]);

			/* read child output */
			while (outpipe[READ_END] > 0 || errpipe[READ_END] > 0) {
				int fdcount, fdmax = 0;
				fd_set readfds;

				FD_ZERO(&readfds);
				if (outpipe[READ_END] > 0) {
					FD_SET(outpipe[READ_END], &readfds);
					if (outpipe[READ_END] > fdmax)
						fdmax = outpipe[READ_END];
				}
				if (errpipe[READ_END] > 0) {
					FD_SET(errpipe[READ_END], &readfds);
					if (errpipe[READ_END] > fdmax)
						fdmax = errpipe[READ_END];
				}
				fdcount = select(fdmax+1, &readfds,
						 NULL, NULL, NULL);
				if (fdcount < 0) {
					if (errno == EINTR)
						continue;
					info("select failed: %s\n",
					    strerror(errno));
					retval = -1;
					break;
				}

				/* get stdout */
				if (outpipe[READ_END] > 0 &&
				    FD_ISSET(outpipe[READ_END], &readfds)) {
					char inbuf[1024];
					char *pos;
					char *line;

					count = read(outpipe[READ_END], inbuf,
						     sizeof(inbuf)-1);
					if (count <= 0) {
						close(outpipe[READ_END]);
						outpipe[READ_END] = -1;
						if (count < 0) {
							err("stdin read failed: %s\n", strerror(errno));
							retval = -1;
						}
						continue;
					}
					inbuf[count] = '\0';

					pos = inbuf;
					if (buf)
						strncpy(buf, inbuf, len);
					while ((line = strsep(&pos, "\n")))
						if (pos && line[0] != '\0')
							warn("%s\n", line);
				}

				/* get stderr */
				if (errpipe[READ_END] > 0 &&
				    FD_ISSET(errpipe[READ_END], &readfds)) {
					char errbuf[1024];
					char *pos;
					char *line;

					count = read(errpipe[READ_END], errbuf,
						     sizeof(errbuf)-1);
					if (count <= 0) {
						close(errpipe[READ_END]);
						errpipe[READ_END] = -1;
						if (count < 0)
							err("stderr read failed: %s\n",
							    strerror(errno));
						continue;
					}
					errbuf[count] = '\0';
					pos = errbuf;
					while ((line = strsep(&pos, "\n")))
						if (pos && line[0] != '\0')
							err("%s\n",line);
				}
			}
			if (outpipe[READ_END] > 0)
				close(outpipe[READ_END]);
			if (errpipe[READ_END] > 0)
				close(errpipe[READ_END]);
		}
		waitpid(child_pid, &status, 0);
		child_pid = 0;
		if (WIFEXITED(status)) {
			retval = WEXITSTATUS(status);
			if (retval != 0)
				info("'%s' returned status %i\n",
				    argv[0], retval);
		} else if (WIFSIGNALED(status)) {
			notice("terminated with signal %d\n",
			     WTERMSIG(status));
			retval = -1;
		} else {
			warn("'%s' abnormal exit\n", argv[0]);
			retval = -1;
		}
	}

	return retval;
}

/*
 * check_dev - Check if the device number matches
 *
 * Check if the device number of the event matches with the
 * device number of the device node and if the device does not
 * has any 'holders' in /sys/block/XX. If not than we can
 * discard this event.
 */
int check_dev(void)
{
	struct stat stbuf;
	char buf[256];
	DIR *dirfd;
	struct dirent *dp;

	if (stat(dev, &stbuf) < 0) {
		fprintf(stderr,"Cannot stat '%s': %d\n", dev, errno);
		return -ENODEV;
	}

	if (!S_ISBLK(stbuf.st_mode)) {
		fprintf(stderr,"Not a block device\n");
		return -ENOTBLK;
	}
	sprintf(buf,"/sys/dev/block/%d:%d/holders", major, minor);
	dirfd = opendir(buf);
	/* Can happen during remove, not an error */
	if (!dirfd)
		return 0;
	while ((dp = readdir(dirfd)) != NULL) {
		if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name, ".."))
			continue;
		fprintf(stderr, "Device %d:%d claimed by %s\n",
			major, minor, dp->d_name);
		closedir(dirfd);
		return -EBUSY;
	}
	closedir(dirfd);

	return 0;
}

/*
 * run_fsck - Run fsck and mount
 *
 * Get an exclusive lock on the lock file and call
 * fsck on the device. On success mount is called.
 * The resulting status is written into the lockfile
 * and the lock is released.
 */
int run_fsck(char *fstype, char *fsopts)
{
	struct flock lock;
	char buf[32];
	char argbuf[32];
	char *argv[9];
	mount_status status = MOUNT_UNCHECKED;
	int rc, i, num;

	sprintf(buf, LOCK_DIR "/%d:%d", major, minor);
	info("Locking file '%s'\n", buf);
	lock_fd = open(buf, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
	if ( lock_fd < 0 ) {
		err("Error opening lock file: %s\n", strerror(errno));
		return errno;
	}
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;

	if (fcntl(lock_fd, F_SETLK, &lock) < 0) {
		err("add: fcntl WRLCK failed: %s\n", strerror(errno));
		return errno;
	}

	/* Check for multipathing */
	info("Check for multipath on '%s'\n", dev);

	memset(buf,0x0,32);
	strcpy(buf,"waiting");
	num = write(lock_fd, buf, 32);

	sprintf(argbuf,"-kshow daemon");
	argv[0] = MPATHD_PROG;
	argv[1] = argbuf;
	argv[2] = NULL;

	rc = run_program(argv, argbuf, 32);
	if (rc < 0) {
		status = MOUNT_FAILED;
	} else {
		if (strlen(argbuf) > 3 && !strncmp(argbuf,"pid", 3)) {
			/*
			 * If multipath is running, it will notify us
			 * again via the 'change' event.
			 * No action required at this point.
			 */
			if (!strncmp(action, "add", 3)) {
				status = MOUNT_SKIPPED;
				goto out_unlock;
			}
		}
	}

	if (passno == 0)
		goto skip_fsck;

	info("Starting fsck on '%s'\n", dev);

	memset(buf, 0x0, 32);
	strcpy(buf,"checking");
	num = write(lock_fd, buf, 32);

	argv[0] = FSCK_PROG;
	argv[1] = "-p";
	argv[2] = "-M";
	i = 3;
	if (fstype) {
		argv[i] = "-t";
		i++;
		argv[i] = fstype;
		i++;
	}
	argv[i] = dev;
	i++;
	argv[i] = NULL;

	rc = run_program(argv, NULL, 0);
	if (rc < 0) {
		status = MOUNT_FAILED;
	} else {
		switch (rc) {
		case 0:
		case 1:
			status = MOUNT_CLEAN;
			break;
		case 2:
			status = MOUNT_CORRECTABLE;
			break;
		case 3:
			status = MOUNT_UNCORRECTED;
			break;
		default:
			status = MOUNT_FAILED;
			break;
		}
	}

	info("fsck done, status %s\n", mount_status_str[status]);

	if (status != MOUNT_CLEAN)
		goto out_unlock;
skip_fsck:
	info("Mounting dev '%s' on '%s'\n", dev, mnt);

	memset(buf, 0x0, 32);
	strcpy(buf, "mounting");
	lseek(lock_fd, 0, SEEK_SET);
	num = write(lock_fd, buf, 32);

	argv[0] = MOUNT_PROG;
	i = 1;
	if (fstype) {
		argv[i] = "-t";
		i++;
		argv[i] = fstype;
		i++;
	}
	if (fsopts) {
		argv[i] = "-o";
		i++;
		argv[i] = fsopts;
		i++;
	}
	argv[i] = dev;
	i++;
	argv[i] = mnt;
	i++;
	argv[i] = NULL;

	rc = run_program(argv, NULL, 0);
	if (rc == 0)
		status = MOUNT_MOUNTED;

	info("mount done (%d), status %s\n", rc, mount_status_str[status]);

out_unlock:
	memset(buf, 0x0, 32);
	strcpy(buf,mount_status_str[status]);
	lseek(lock_fd, 0, SEEK_SET);
	num = write(lock_fd, buf, 32);

	lock.l_type = F_UNLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;

	if (fcntl(lock_fd, F_SETLK, &lock) < 0) {
		err("fcntl UNLCK failed: %s\n", strerror(errno));
	}
	close(lock_fd);
	lock_fd = -1;
	return 0;
}

/*
 * daemonize_fsck - background the fsck & mount process
 *
 * Running fsck and mount in the background. Copied
 * from Stevens' Advanced Programming.
 */
int daemonize_fsck(char *fstype, char *fsopts)
{
	pid_t pid;
	int status;
	int i;
	struct rlimit rl;
	struct sigaction sa;

	umask(0);
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
		fprintf(stderr,"Can't get file limit\n");
		return errno;
	}
	if ((pid = fork()) < 0) {
		fprintf(stderr,"1st fork error: %s\n", strerror(errno));
		return errno;
	} else if (pid == 0) { /* first child */
		int fd;

		setsid();
		sa.sa_handler = SIG_IGN;
		sigemptyset(&sa.sa_mask);
		if (sigaction(SIGHUP, &sa, NULL) < 0) {
			fprintf(stderr, "can't ignore SIGHUP\n");
			exit(errno);
		}
		if ((pid = fork()) < 0) {
			fprintf(stderr, "2nd fork error: %s\n",
				strerror(errno));
			return errno;
		} else if (pid > 0) {
			exit(0);
		}
		status = chdir("/");
		if (rl.rlim_max == RLIM_INFINITY)
			rl.rlim_max = 1024;

		for (i = 0; i < rl.rlim_max; i++)
			close(i);

		fd = open("/dev/null", O_RDWR);
		fd = dup(0);
		fd = dup(0);
		openlog("udev.mountd", 0, LOG_DAEMON);
		run_fsck(fstype, fsopts);
		closelog();
		exit(0);
	}

	if (waitpid(pid, &status, 0) != pid)
		fprintf(stderr,"waitpid error: %s\n", strerror(errno));

	exit(WEXITSTATUS(status));
}

/*
 * get_lock - Examine the lockfile
 *
 * Try to get a lock on the lockfile. If this fails the
 * background process is already running and we return
 * the PID of the background process in *pid.
 * If we can get a lock we read the current status from
 * the lockfile and return a pid of 0 as no process is
 * currently running.
 */
int get_lock(pid_t *pid, char *buf, unsigned int buflen)
{
	struct flock lock;
	int fd;
	char path[256];

	sprintf(path, LOCK_DIR "/%d:%d", major, minor);
	fd = open(path, O_RDONLY);
	if ( fd < 0 ) {
		if (errno == ENOENT) {
			sprintf(buf,"unknown");
			*pid = 0;
			return 0;
		}
		return errno;
	}
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;

	if (fcntl(fd, F_GETLK, &lock) < 0)
		goto out;
	if (lock.l_type == F_UNLCK) {
		int num;

		num = read(fd, buf, buflen);
		if (num < 0)
			goto out;
		buf[num] = '\0';
		*pid = 0;
	} else {
		*pid = lock.l_pid;
	}
	errno = 0;
out:
	close(fd);

	return errno;
}

/*
 * remove_lock - Remove the lockfile
 *
 * When called with 'remove' we should remove the lockfile
 * as the device is gone and hence all status is invalid.
 */
int remove_lock(void)
{
	struct stat stbuf;
	char path[256];

	sprintf(path,LOCK_DIR "/%d:%d", major, minor);
	if (stat(path, &stbuf) < 0) {
		/* That's okay when called twice with 'remove' */
		return 0;
	}
	if (unlink(path) < 0)
		fprintf(stderr, "Cannot remove lockfile: %d\n", errno);

	return errno;
}

/*
 * check_mnt - Check if the device is mounted
 *
 * Compare the device number from the event
 * with the device number of the device containing
 * the mount directory. If they are identical then
 * the device is already mounted.
 */
int check_mnt(void)
{
	struct stat stbuf;

	if (stat(mnt, &stbuf) < 0) {
		fprintf(stderr,"Cannot stat '%s': %d\n", mnt, errno);
		return -1;
	}

	if (!S_ISDIR(stbuf.st_mode)) {
		fprintf(stderr,"Not a directory\n");
		return -1;
	}

	return (stbuf.st_dev != makedev(major,minor));
}

int create_lockdir(void)
{
	struct stat stbuf;

	if (stat(LOCK_DIR, &stbuf) == 0) {
		if (S_ISDIR(stbuf.st_mode))
			return 0;

		/* Exists, but not a directory. Try to remove. */
		if (unlink(LOCK_DIR) < 0) {
			fprintf(stderr,"Cannot remove stale file '%s': %s\n"
				"Please remove manually\n", LOCK_DIR,
				strerror(errno));
			return errno;
		}
	}
	/* Try to create lock directory */
	if (mkdir(LOCK_DIR, S_IRWXU|S_IRGRP|S_IXGRP) < 0) {
		fprintf(stderr,"Cannot create lock directory %s: %s\n",
			LOCK_DIR, strerror(errno));
		return errno;
	}
	return 0;
}

int main(int argc, char **argv, char **envp)
{
	mount_ops op = MOUNT_UNKNOWN;
	char *ep, *fstype = NULL, *fsopts = NULL, *ptr;
	int err, pid;
	char buf[64];

	/* Check if lockfile directory exists */
	if (create_lockdir())
		return ENOENT;

	if (argc < 2)
		op = MOUNT_CHECK;
	else {
		if (!strncmp(argv[1], "add", 3))
			op = MOUNT_FSCK;
		else if (!strncmp(argv[1], "remove", 6))
			op = MOUNT_REMOVE;
		else if (!strncmp(argv[1], "check", 5))
			op = MOUNT_CHECK;
	}
	if (op == MOUNT_UNKNOWN) {
		fprintf(stderr, "No operation found\n");
		return EINVAL;
	}

	/* Scan environment */
	while (envp && *envp) {
		ep = *envp;
		if (!strncmp(ep, "ACTION", 6) && strlen(ep) > 7) {
			action = malloc(strlen(ep + 7) + 1);
			if (!action)
				return ENOMEM;
			strcpy(action, ep + 7);
		}
		if (!strncmp(ep, "MAJOR", 5) && strlen(ep) > 6) {
			major = strtoul(ep + 6, &ptr, 10);
			if (ptr == ep + 6) {
				fprintf(stderr,"Invalid major number '%s'\n",
					ep + 6);
				major = 0;
			}
		}
		if (!strncmp(ep, "MINOR", 5) && strlen(ep) > 6) {
			minor = strtoul(ep + 6, &ptr, 10);
			if (ptr == ep + 6) {
				fprintf(stderr,"Invalid minor number '%s'\n",
					ep + 6);
				minor = 0;
			}
		}
		if (!strncmp(ep, "UDEV_LOG", 8) && strlen(ep) > 9) {
			logging = strtoul(ep + 9, &ptr, 10);
			if (ptr == ep + 9) {
				fprintf(stderr,"Invalid logging level '%s'\n",
					ep + 6);
				logging = DEFAULT_LOGLEVEL;
			}
			if (logging > LOG_DEBUG)
				logging = LOG_DEBUG;
		}
		if (!strncmp(ep,"FSTAB_NAME", 10) && strlen(ep) > 11) {
			dev = malloc(strlen(ep + 11) + 1);
			if (!dev)
				return ENOMEM;
			strcpy(dev, ep + 11);
		}
		if (!strncmp(ep, "FSTAB_DIR", 9) && strlen(ep) > 10) {
			mnt = malloc(strlen(ep + 10) + 1);
			if (!mnt)
				return ENOMEM;
			strcpy(mnt, ep + 10);
		}
		if (!strncmp(ep, "FSTAB_OPTS", 10) && strlen(ep) > 11)
			fsopts = ep + 11;
		if (!strncmp(ep, "FSTAB_TYPE", 10) && strlen(ep) > 11)
			fstype = ep + 11;
		if (!strncmp(ep, "FSTAB_PASSNO", 12) && strlen(ep) > 13) {
			passno = strtoul(ep + 13, &ptr, 10);
			if (ptr == ep + 13) {
				fprintf(stderr,"Invalid passno '%s'\n",
					ep + 13);
				passno = 0;
			}
		}
		envp++;
	}
	/* Sanity checks */
	if (!dev || !strlen(dev)) {
		fprintf(stderr,"No device given\n");
		return EINVAL;
	}
	if (!mnt || !strlen(mnt)) {
		fprintf(stderr, "Invalid mount point\n");
		return EINVAL;
	}
	if (major == 0) {
		fprintf(stderr, "Invalid major number\n");
		return EINVAL;
	}

	if (!fstype || !strlen(fstype))
		fprintf(stderr, "No fs type given\n");

	/* Check the lock file */
	err = get_lock(&pid, buf, 64);
	if (err) {
		fprintf(stderr,"Cannot get lock file: %d\n", err);
		return EACCES;
	}

	if (pid > 0 && op != MOUNT_REMOVE) {
		/* No error, checking in progress */
		if (op == MOUNT_CHECK)
			fprintf(stdout,"FSCK_STATE=running\n");
		return EBUSY;
	}

	/* Check for matching device */
	err = check_dev();
	if (err < 0) {
		if (err == -ENODEV && op == MOUNT_CHECK) {
			fprintf(stdout,"FSCK_STATE=unknown\n");
			return 0;
		}
		if (err == -ENOENT && op == MOUNT_REMOVE)
			goto skip_mount;

		return -err;
	}

	/* Check if the device is mounted */
	err = check_mnt();
	if (err < 0) {
		if (err == -EINVAL && op == MOUNT_FSCK) {
			fprintf(stdout,"FSCK_STATE=skipped\n");
			return 0;
		}
		return EINVAL;
	}

skip_mount:
	/* The real action */
	if (op == MOUNT_FSCK) {
		if (err == 0) {
			/* Already mounted */
			return 0;
		}
		err = daemonize_fsck(fstype, fsopts);
		if (err) {
			fprintf(stderr,"Cannot execute fsck: %d\n", err);
			return err;
		}
	} else if (op == MOUNT_CHECK) {
		if (err > 0 && !strncmp(buf, "mounted", 7)) {
			/* Unmounted by user */
			strcpy(buf, "clean");
		}
		fprintf(stdout,"FSCK_STATE=%s\n", buf);
	} else {
		if (pid > 0) {
			char file[256];
			struct stat stbuf;
			int timeout = SIGNAL_TIMEOUT;

			kill(pid, SIGTERM);
			/* wait for the process to terminate */
			sprintf(file,"/proc/%d", pid);
			while ( timeout > 0 && stat(file,&stbuf) >=0 ) {
				sleep(1);
				timeout--;
			}
			if (timeout == 0 && stat(file,&stbuf) >= 0) {
				return EBUSY;
			}
		}
		if (err == 0) {
			/* Already mounted */
			return EBUSY;
		}
		remove_lock();
	}
	return 0;
}
