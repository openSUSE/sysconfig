/*
 * autoip: a zeroconf local link ip address configuration tool
 *
 * Copyright (c) 2004 SuSE Linux AG
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 ****************************************************************
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <asm/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/filter.h>
#include <linux/if_arp.h>


/* some globals so that we can work in our signal handler */
char statusfile[256];
char *myprgname;
char *myifname;
unsigned char myip[4];
int sock4;
int debug;

/* filters, test for arp "just in case" */

/* probe filter */
/* arp and (arp[14:4]=0x00112233 or (arp[24:4]=0x00112233 and (arp[8:4]!=0x8899aabb or arp[12:2]!=0xccdd))) */
struct sock_filter probe_filter[] = {
  { 0x28, 0, 0, 0x0000000c },
  { 0x15, 0, 9, 0x00000806 },
  { 0x20, 0, 0, 0x0000001c },
  { 0x15, 6, 0, 0x00112233 },
  { 0x20, 0, 0, 0x00000026 },
  { 0x15, 0, 5, 0x00112233 },
  { 0x20, 0, 0, 0x00000016 },
  { 0x15, 0, 2, 0x8899aabb },
  { 0x28, 0, 0, 0x0000001a },
  { 0x15, 1, 0, 0x0000ccdd },
  { 0x6,  0, 0, 0x00000060 },
  { 0x6,  0, 0, 0x00000000 },
};

/* defend filter */
/* arp and (arp[14:4]=0x00112233 and (arp[8:4]!=0x8899aabb or arp[12:2]!=0xccdd)) */
struct sock_filter defend_filter[] = {
  { 0x28, 0, 0, 0x0000000c },
  { 0x15, 0, 7, 0x00000806 },
  { 0x20, 0, 0, 0x0000001c },
  { 0x15, 0, 5, 0x00112233 },
  { 0x20, 0, 0, 0x00000016 },
  { 0x15, 0, 2, 0x8899aabb },
  { 0x28, 0, 0, 0x0000001a },
  { 0x15, 1, 0, 0x0000ccdd },
  { 0x6,  0, 0, 0x00000060 },
  { 0x6,  0, 0, 0x00000000 },
};


void
read_status(unsigned char *ip, int *pid, char *status, int statuslen)
{
  char buf[256], *bp;
  int fd, l, i;
  in_addr_t addr;
  uint32_t haddr;

  if (ip)
    memset(ip, 0, 4);
  if (pid)
    *pid = 0;
  if (status && statuslen > 0)
    *status = 0;
  if ((fd = open(statusfile, O_RDONLY)) == -1)
    return;
  l = read(fd, buf, sizeof(buf));
  close(fd);
  bp = buf;
  while (l > 0)
    {
      for (i = 0; i < l; i++)
	if (bp[i] == '\n')
	  break;
      if (i == l)
	return;
      bp[i] = 0;
      if (ip && !strncmp(bp, "IPADDR=", 7))
	{
	  if ((addr = inet_addr(buf + 7)) != INADDR_NONE)
	    {
	      haddr = ntohl(addr);
	      if (haddr >= 0xa9fe0100 && haddr < 0xa9feff00)
		{
		  ip[0] = haddr >> 24 & 0xff;
		  ip[1] = haddr >> 16 & 0xff;
		  ip[2] = haddr >> 8 & 0xff;
		  ip[3] = haddr      & 0xff;
		}
	    }
	}
      else if (pid && !strncmp(bp, "PID=", 4))
	{
	  *pid = atoi(bp + 4);
	}
      else if (status && !strncmp(bp, "STATUS=", 7))
	{
	  if (i - 7 < statuslen)
	    memcpy(status, bp + 7, i - 7 + 1);
	}
      bp += i + 1;
      l -= i + 1;
    }
}

void
write_status(unsigned char *ip, char *status, int pid)
{
  char buf[256];
  char *bp = buf;
  int fd;

  if (ip[0])
    {
      sprintf(bp, "IPADDR=%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
      bp += strlen(bp);
    }
  if (pid && bp + 4 + 10 < buf + sizeof(buf))
    {
      sprintf(bp, "PID=%d\n", pid);
      bp += strlen(bp);
    }
  if (status && bp + 8 + strlen(status) < buf + sizeof(buf))
    {
      sprintf(bp, "STATUS=%s\n", status);
      bp += strlen(bp);
    }
  if ((fd = open(statusfile, O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1)
  if ((fd = open(statusfile, O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1)
    return;
  write(fd, buf, bp - buf);
  close(fd);
}

void
die(char *str, int err)
{
  if (str)
    {
      if (err)
	{
	  errno = err;
	  perror(str);
	}
      else
	fprintf(stderr, "%s\n", str);
    }
  write_status(myip, "FAILED", 0);
  exit(1);
}

void
install_filter(int sock, struct sock_filter *fil, int filn, unsigned char *ip, unsigned char *hw)
{
  struct sock_fprog fprog;
  struct sock_filter *f2;
  int i;
  char buf[1024];

  if ((f2 = (struct sock_filter *)malloc(sizeof(*f2) * filn)) == 0)
    die("no memory for filter!", 0);
  for (i = 0; i < filn; i++)
    {
      f2[i] = fil[i];
      if (f2[i].k == 0x00112233)
        f2[i].k = ip[0] << 24 | ip[1] << 16 | ip[2] << 8 | ip[3];
      if (f2[i].k == 0x889911bb)
        f2[i].k = hw[0] << 24 | hw[1] << 16 | hw[2] << 8 | hw[3];
      if (f2[i].k == 0xccdd)
        f2[i].k = hw[4] << 8 | hw[5];
    }
  /* install filter */
  fprog.len = filn;
  fprog.filter = f2;
  if (setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &fprog, sizeof(fprog)))
    die("filter setsockopt", errno);
  free(f2);

  /* now drain old leftovers */
  while (recv(sock, buf, sizeof(buf), 0) > 0)
    ;
}

unsigned char nullip[] = {0, 0, 0, 0};
unsigned char nullhw[] = {0, 0, 0, 0, 0, 0};
unsigned char broadhw[] = {255, 255, 255, 255, 255, 255};

void
mkarp(unsigned char *arp, int ifhrd, unsigned char *sendhw, unsigned char *sendip, unsigned char *targhw, unsigned char *targip)
{
  memset(arp + 0, 255, 6);	/* always broadcast */
  memmove(arp + 6, sendhw, 6);
  arp[12] = ETHERTYPE_ARP >> 8;
  arp[13] = ETHERTYPE_ARP & 255;
  arp[14] = ifhrd >> 8;
  arp[15] = ifhrd & 255;
  arp[16] = ETHERTYPE_IP >> 8;
  arp[17] = ETHERTYPE_IP & 255;
  arp[18] = 6;
  arp[19] = 4;
  arp[20] = 0;
  arp[21] = 1;
  memmove(arp + 22, sendhw, 6);
  memmove(arp + 28, sendip, 4);
  memmove(arp + 32, targhw, 6);
  memmove(arp + 38, targip, 4);
}

void
check_if(int sock, char *ifname, unsigned char *ip)
{
  struct ifreq ifreq;
  unsigned char *iip;

  memset(&ifreq, 0, sizeof(ifreq));
  strcpy(ifreq.ifr_name, ifname);
  if (ioctl(sock, SIOCGIFADDR, &ifreq) && errno != EADDRNOTAVAIL)
    die("SIOCGIFADDR ioctl", errno);
  iip = (unsigned char *)&((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
  if (memcmp(ip, iip, 4))
    {
      if (debug)
	{
	  printf("ip mismatch %d.%d.%d.%d - %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3], iip[0], iip[1], iip[2], iip[3]);
	  printf("goodbye...\n");
	}
      write_status(myip, "BEATEN", 0);
      exit(0);
    }
}

int
check_if_good(int sock, char *ifname, int check)
{
  struct ifreq ifreq;
  unsigned char *ip;

  memset(&ifreq, 0, sizeof(ifreq));
  strcpy(ifreq.ifr_name, ifname);
  if (ioctl(sock, SIOCGIFADDR, &ifreq) && errno != EADDRNOTAVAIL)
    die("SIOCGIFADDR ioctl", errno);
  ip = (unsigned char *)&((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
  if ((ip[0] || ip[1] || ip[2] || ip[3]) && ip[0] != 169 && ip[1] != 254)
    {
      if (debug)
        printf("%s already configured: %d.%d.%d.%d\n", ifname, ip[0], ip[1], ip[2], ip[3]);
      if (check)
        return 0;
      if (debug)
        printf("goodbye...\n");
      write_status(myip, "BEATEN", 0);
      exit(0);
    }
  return 1;
}

void
ifconf(int sock, char *ifname, int f, unsigned char *ip, unsigned int b, unsigned int n)
{
  struct ifreq ifreq;

  memset(&ifreq, 0, sizeof(ifreq));
  strcpy(ifreq.ifr_name, ifname);
  ifreq.ifr_addr.sa_family = AF_INET;
  memmove((unsigned char *)&((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr, ip, 4);
  if (ioctl(sock, SIOCSIFADDR, &ifreq))
    die("SIOCSIFADDR ioctl", errno);
  if (((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr)
    {
      ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr = htonl(b);
      if (ioctl(sock, SIOCSIFBRDADDR, &ifreq))
	die("SIOCSIFBRDADDR ioctl", errno);
      ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr = htonl(n);
      if (ioctl(sock, SIOCSIFNETMASK, &ifreq))
	die("SIOCSIFNETMASK ioctl", errno);
    }
  if (ioctl(sock, SIOCGIFFLAGS, &ifreq))
    die("SIOCGIFFLAGS ioctl", errno);
  /* we leave the interface up */
  /* ifreq.ifr_flags &= ~(IFF_UP | IFF_BROADCAST | IFF_MULTICAST); */
  ifreq.ifr_flags |= f;
  if (ioctl(sock, SIOCSIFFLAGS, &ifreq))
    die("SIOCSIFFLAGS ioctl", errno);
}

void
nextip(unsigned char *ip)
{
  unsigned int r = rand();
#if RAND_MAX >= 16777216
  r >>= 8;	/* don't use lower bits */
#endif
  r %= 256 * 254;
  ip[0] = 169;
  ip[1] = 254;
  ip[2] = 1 + (r >> 8);
  ip[3] = r & 255;
}


void
sigterm(int sig)
{
  check_if_good(sock4, myifname, 0);
  if (*myip)
    check_if(sock4, myifname, myip);
  ifconf(sock4, myifname, 0, nullip, 0x00000000, 0x00000000);
  write_status(myip, 0, 0);
  exit(0);
}

pid_t
toback(unsigned char *myip, char *status)
{
  int pi[2];
  char c;
  pid_t pid;

  if (!status || pipe(pi) == -1)
    pi[0] = pi[1] = -1;
  pid = fork();
  if (pid == (pid_t)-1)
    die("fork", errno);
  if (pid)
    {
      if (pi[0])
	{
	  /* wait until child has updated status */
	  close(pi[1]);
	  while (read(pi[0], &c, 1) == -1 && errno == EINTR)
	    ;
	}
      return pid;
    }
  pid = getpid();
  if (status)
    write_status(myip, status, (int)pid);
  if (pi[0])
    {
      /* signal parent that we have updated the status */
      close(pi[0]);
      close(pi[1]);
    }
  setsid();
  close(0);
  close(1);
  close(2);
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_WRONLY);
  open("/dev/null", O_WRONLY);
  return 0;
}

void
do_query(int sock, char *ifname)
{
  int pid;
  char status[64];

  read_status(0, &pid, status, sizeof(status));
  if (!check_if_good(sock, ifname, 1))
    {
      printf("BEATEN\n");
      return;
    }
  if (!strcmp(status, "BEATEN"))
    strcpy(status, "INACTIVE");
  if (pid)
    {
      /* validate that the pid belongs to us */
      char procpath[128];
      char exebuf[256];
      int l;

      sprintf(procpath, "/proc/%d/exe", pid);
      l = readlink(procpath, exebuf, sizeof(exebuf) - 1);
      if (l > 0 && l < sizeof(exebuf))
	{
	  exebuf[l] = 0;
	  if (!strstr(exebuf, myprgname))
	    pid = 0;
	}
    }
  if (!*status || (pid && kill(pid, 0) == ESRCH))
    {
      printf("INACTIVE\n");
      return;
    }
  printf("%s\n", status);
}

void
usage(void)
{
  fprintf(stderr, "Usage: autoip [-p] [-f] [-d] [-b] [-B] [-g grace] [-m min] [-M max] <interface>\n");
  exit(1);
}

int
main(int argc, char **argv)
{
  int sock, c;
  char buf[256];
  int l, ifindex, ifhrd;
  struct ifreq ifreq;
  struct packet_mreq mreq;
  struct sockaddr_ll sall;
  int promisc = 0;
  unsigned char myhwaddr[6];
  unsigned char arp[14 + 28];
  struct timeval tv;
  fd_set fds;
  int probe_min = 1 * 1000;
  int probe_max = 2 * 1000;
  int probecnt;
  int conflictcnt;
  int force = 0;
  time_t lastconflict, now;
  int background = 0;
  int pid = 0;
  int query = 0;
  unsigned long int grace = 0;

  myprgname = strrchr(argv[0], '/');
  if (!myprgname || *++myprgname == 0)
    myprgname = argv[0];
  while ((c = getopt(argc, argv, "fpqdbBg:m:M:")) != -1)
    {
      switch (c)
	{
	case 'q':
	  query = 1;
	  break;
	case 'd':
	  debug = 1;
	  break;
	case 'f':
	  force = 1;
	  break;
	case 'p':
	  promisc = 1;
	  break;
	case 'b':
	  background = 1;
	  break;
	case 'B':
	  background = 2;
	  break;
	case 'g':
	  {
	    char *endptr;
	    grace = strtoul(optarg, &endptr, 10);
	    if (*endptr != '\0')
	      usage();
	  }
	  break;
	case 'm':
	  probe_min = atoi(optarg);
	  break;
	case 'M':
	  probe_max = atoi(optarg);
	  break;
	default:
	  usage();
	}
    }
  argc -= optind - 1;
  argv += optind - 1;

  pid = getpid();

  if (argc != 2)
    usage();
  if (probe_min > probe_max)
    {
      fprintf(stderr, "bad probe timings\n");
      exit(1);
    }

  myifname = argv[1];

  if ((sock4 = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
      perror("inet socket");
      exit(1);
    }

  /* find interface index */
  memset(&ifreq, 0, sizeof(ifreq));
  strcpy(ifreq.ifr_name, myifname);
  if (ioctl(sock4, SIOCGIFINDEX, &ifreq))
    {
      fprintf(stderr, "autoip: %s: unknown interface\n", myifname);
      exit(1);
    }
  ifindex = ifreq.ifr_ifindex;

  if (strlen(myifname) + 30 > sizeof(statusfile))
    exit(1);
  sprintf(statusfile, "/var/lib/autoip/autoip-%s.info", myifname);
  if (query)
    {
      do_query(sock4, myifname);
      exit(0);
    }
  read_status(myip, 0, 0, 0);
  write_status(myip, "CHOOSING", background == 2 ? 0 : pid);
  if (background == 2)
    {
      if (toback(myip, "CHOOSING"))
	_exit(0);
      background = 0;
      pid = getpid();
    }

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
  grace *= 40;
  if (!force) while (grace-- > 0)
    {
      const struct timespec req = {0, 25000000};
      check_if_good(sock4, myifname, 0);
      nanosleep(&req, NULL);
    }
#else
  while (grace > 0)
    {
      if (!force)
	check_if_good(sock4, myifname, 0);
      sleep(1);
      grace--;
    }
#endif
    
  if (!force)
    check_if_good(sock4, myifname, 0);

  /* bring the interface up */
  if (ioctl(sock4, SIOCGIFFLAGS, &ifreq))
    die("SIOCGIFFLAGS ioctl", errno);
  ifreq.ifr_flags |= IFF_UP;
  if (ioctl(sock4, SIOCSIFFLAGS, &ifreq))
    die("SIOCSIFFLAGS ioctl", errno);
  
  /* install term handler */
  signal(SIGTERM, sigterm);

  if ((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) == -1)
    die("packet socket", errno);
  if (fcntl(sock, F_SETFL, O_NONBLOCK))
    die("NONBLOCK fcntl", errno);

  /* get hw address */
  strcpy(ifreq.ifr_name, myifname);
  if (ioctl(sock, SIOCGIFHWADDR, &ifreq))
    die("SIOCGIFHWADDR ioctl", errno);
  ifhrd = ifreq.ifr_hwaddr.sa_family;
  if (ifhrd != ARPHRD_ETHER)
    {
      fprintf(stderr, "autoip: %s: not an ethernet device\n", myifname);
      die((char *)0, 0);
    }
  memmove(myhwaddr, ifreq.ifr_hwaddr.sa_data, 6);
  if (debug)
    printf("my addr %02x:%02x:%02x:%02x:%02x:%02x\n", myhwaddr[0], myhwaddr[1], myhwaddr[2], myhwaddr[3], myhwaddr[4], myhwaddr[5]);

  /* bind socket to interface */
  memset(&sall, 0, sizeof(sall));
  sall.sll_family = AF_PACKET;
  sall.sll_protocol = 0;
  sall.sll_ifindex = ifindex;
  if (bind(sock, (struct sockaddr *)&sall, sizeof(sall)))
    die("bind", errno);

  if (promisc)
    {
      memset(&mreq, 0, sizeof(mreq));
      mreq.mr_ifindex = ifindex;
      mreq.mr_type = PACKET_MR_PROMISC;
      if (setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)))
	die("PACKET_MR_PROMISC setsockopt", errno);
    }

  srand(myhwaddr[2] << 24 | myhwaddr[3] << 16 | myhwaddr[4] << 8 | myhwaddr[5]);

  /* check if we can use the already configured ip address */
  if (ioctl(sock4, SIOCGIFADDR, &ifreq) == 0)
    {
      unsigned char *iip = (unsigned char *)&((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
      if (iip[0] == 169 && iip[1] == 254 && (iip[2] > 0 && iip[2] < 255))
	memcpy(myip, iip, 4);
    }

  /* if we still have no address, roll a brand new one */
  if (!myip[0])
    nextip(myip);

  for (;;)
    {
      /* first probe for a conflict */
      write_status(myip, "CHOOSING", pid);
      for (conflictcnt = 0; ; conflictcnt++)
	{
	  if (debug)
	    printf("trying %d.%d.%d.%d\n", myip[0], myip[1], myip[2], myip[3]);

	  install_filter(sock, probe_filter, sizeof(probe_filter)/sizeof(*probe_filter), myip, myhwaddr);

	  for (probecnt = 0; probecnt < 3; probecnt++)
	    {
	      if (debug)
	        printf("sending probe\n");
	      mkarp(arp, ifhrd, myhwaddr, nullip, nullhw, myip);
	      if ((l = send(sock, arp, 42, 0)) != 42)
		die("arpprobe send", errno);
	      l = probecnt == 2 ? probe_max : probe_min + ((unsigned int)rand()) % (probe_max - probe_min);
	      tv.tv_sec = l / 1000;
	      tv.tv_usec = (l % 1000) * 1000;
	      FD_ZERO(&fds);
	      FD_SET(sock, &fds);
	      if ((l = select(sock + 1, &fds, 0, 0, &tv)) == -1)
		die("select ", errno);
	      if (l)
		break;
	    }
	  if (!force)
	    check_if_good(sock4, myifname, 0);
	  if (probecnt == 3)
	    break;
	  if (debug)
	    printf("conflict, trying next ip!\n");
	  if (conflictcnt == 10)
	    sleep(60);
	  nextip(myip);
	}

      /* no conflict, configure address and defend it */
      if (debug)
        printf("using %d.%d.%d.%d\n", myip[0], myip[1], myip[2], myip[3]);
      ifconf(sock4, myifname, IFF_UP|IFF_BROADCAST|IFF_MULTICAST, myip, 0xa9feffff, 0xffff0000);

      if (background)
	{
	  if (toback(myip, "DEFENDING"))
	    {
	      printf("IPADDR=%d.%d.%d.%d\n", myip[0], myip[1], myip[2], myip[3]);
	      fflush(stdout);
	      _exit(0);
	    }
	  background = 0;
	  pid = getpid();
	}
      write_status(myip, "DEFENDING", pid);

      install_filter(sock, defend_filter, sizeof(defend_filter)/sizeof(*probe_filter), myip, myhwaddr);


      /* announce new address */
      mkarp(arp, ifhrd, myhwaddr, myip, myhwaddr, myip);
      if ((l = send(sock, arp, 42, 0)) != 42)
	die("arpprobe send", errno);
      FD_ZERO(&fds);
      FD_SET(sock, &fds);
      tv.tv_sec = probe_max / 1000;
      tv.tv_usec = (probe_max % 1000) * 1000;
      if ((l = select(sock + 1, &fds, 0, 0, &tv)) == -1)
	die("select", errno);
      if (l == 0 && (l = send(sock, arp, 42, 0)) != 42)
	die("arpprobe send", errno);

      /* watch for conflicts and defend */
      lastconflict = 0;
      for (;;)
	{
	  check_if(sock4, myifname, myip);
	  tv.tv_sec = 10;
	  tv.tv_usec = 0;
	  FD_ZERO(&fds);
	  FD_SET(sock, &fds);
	  if ((l = select(sock + 1, &fds, 0, 0, &tv)) == -1)
	    die("select", errno);
          if (l == 0)
	    continue;
	  if (debug)
	    printf("conflict!\n");
	  while (recv(sock, buf, sizeof(buf), 0) > 0)
	    ;
	  time(&now);
	  if (lastconflict && now - lastconflict < 10)
	    break;
	  if (debug)
	    printf("defending...\n");
	  if ((l = send(sock, arp, 42, 0)) != 42)
	    die("arpprobe send", errno);
	  lastconflict = now;
	}
      ifconf(sock4, myifname, 0, nullip, 0x00000000, 0x00000000);
      nextip(myip);
    }
}
