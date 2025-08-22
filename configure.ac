# Process this file with autoconf to produce a configure script.
AC_INIT([sysconfig], esyscmd([tr -d '\n' < VERSION]),
	[http://bugs.opensuse.org], [sysconfig],
	[https://github.com/openSUSE/sysconfig])

VERSION=$(cat ${srcdir}/VERSION)
AC_SUBST(VERSION)

AM_INIT_AUTOMAKE([foreign dist-bzip2 no-dist-gzip])
AC_CONFIG_SRCDIR([config/sysconfig.config-network.in])
AC_REVISION($Revision: 1.13 $)

AC_PREFIX_DEFAULT(/usr)

# File locations
sysconfigdir='${sysconfdir}/sysconfig'
sysconfig_hardwaredir='${sysconfdir}/sysconfig/hardware'
sysconfig_networkdir='${sysconfdir}/sysconfig/network'
sysconfig_network_scriptsdir='${sysconfdir}/sysconfig/network/scripts'
init_ddir='${sysconfdir}/init.d'
docdir='${prefix}/usr/share/doc/packages/sysconfig'
nm_dispatcherdir='${sysconfdir}/NetworkManager/dispatcher.d'
modprobedir='${sysconfdir}/modprobe.d'
sysconf_pppdir='${sysconfdir}/ppp'
libexec_pppdir='${libexecdir}/ppp'
libexec_netconfigdir='${libexecdir}/netconfig'
libexec_netconfig_d_scriptsdir='${libexec_netconfigdir}/netconfig.d'
sysconf_netconfig_d_scriptsdir='${sysconfdir}/netconfig.d'
fillup_templatesdir='${prefix}/var/adm/fillup-templates'
AC_SUBST(sysconfigdir)
AC_SUBST(sysconfig_hardwaredir)
AC_SUBST(sysconfig_networkdir)
AC_SUBST(sysconfig_network_scriptsdir)
AC_SUBST(init_ddir)
AC_SUBST(docdir)
AC_SUBST(sysconf_pppdir)
AC_SUBST(libexec_pppdir)
AC_ARG_WITH([unitdir], [AS_HELP_STRING([--with-unitdir@<:@=PATH@:>@],
	    [path to the systemd/system unit directory])],,
	    [with_unitdir='${prefix}/lib/systemd/system'])
AC_SUBST(unitdir, $with_unitdir)
AC_ARG_WITH([udevdir], [AS_HELP_STRING([--with-udevdir@<:@=PATH@:>@],
	    [path to the udev installation directory])],,
	    [with_udevdir='${prefix}/lib/udev'])
AC_SUBST(udevdir, $with_udevdir)
AC_SUBST(udevrulesdir, ${with_udevdir}/rules.d)
AC_SUBST(modprobedir)
AC_SUBST(nm_dispatcherdir)
AC_SUBST(libexec_netconfigdir)
AC_SUBST(libexec_netconfig_d_scriptsdir)
AC_SUBST(sysconf_netconfig_d_scriptsdir)
AC_ARG_WITH(fillup-templatesdir,
	[  --with-fillup-templatesdir=name   Set the pathname for the fillup templates directory],[
	fillup_templatesdir=$withval; AC_MSG_RESULT(configuration directory: $fillup_templatesdir)],
	[fillup_templatesdir='${prefix}/var/adm/fillup-templates'; AC_MSG_RESULT(fillup templates directory: $fillup_templatesdir)
])
AC_SUBST(fillup_templatesdir)

AC_ARG_ENABLE([netconfig-dns],
	[AS_HELP_STRING([--disable-netconfig-dns],
	[disable netconfig dns updater modules])],,
	[enable_netconfig_dns=yes])
AM_CONDITIONAL([netconfig_dns], [test "x$enable_netconfig_dns" = "xyes"])

AC_ARG_ENABLE([netconfig-ntp],
	[AS_HELP_STRING([--disable-netconfig-ntp],
	[disable netconfig ntp updater modules])],,
	[enable_netconfig_ntp=yes])
AM_CONDITIONAL([netconfig_ntp], [test "x$enable_netconfig_ntp" = "xyes"])

AC_ARG_ENABLE([netconfig-nis],
	[AS_HELP_STRING([--disable-netconfig-nis],
	[disable netconfig nis updater modules])],,
	[enable_netconfig_nis=yes])
AM_CONDITIONAL([netconfig_nis], [test "x$enable_netconfig_nis" = "xyes"])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h errno.h netdb.h netinet/in.h stdlib.h string.h unistd.h])



# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostbyname strerror])


AC_CONFIG_FILES([Makefile
		 doc/Makefile
		 config/Makefile
		 config/hardware/Makefile
		 tools/Makefile
		 sysconfig.spec
		 scripts/Makefile
		 scripts/ppp/Makefile
		 scripts/netconfig.d/Makefile
		 scripts/hardware/Makefile])
AC_OUTPUT

