## Process this file with automake to produce Makefile.in

EXTRA_DIST = 	VERSION \
		package/sysconfig.spec.in 

AUTOMAKE_OPTIONS = foreign dist-bzip2

CLEANFILES = *~

SUBDIRS = doc scripts tools config package

$(srcdir)/configure: configure.in aclocal.m4 VERSION
	cd $(srcdir) && autoconf


dist-hook: package/sysconfig.spec VERSION cl

sysconfig-$(VERSION).tar.bz2: all dist-bzip2 

pac-poeml: sysconfig-$(VERSION).tar.bz2
	mkdir -p $$HOME/pac/@PACKAGE@
	cp -p @PACKAGE@-$(VERSION).tar.bz2 \
		package/sysconfig.spec \
		package/sysconfig.changes \
		$$HOME/pac/@PACKAGE@/

pac-zoz: sysconfig-$(VERSION).tar.bz2
	mkdir -p $$HOME/@PACKAGE@
	cp -p @PACKAGE@-$(VERSION).tar.bz2 \
		package/sysconfig.spec \
		package/sysconfig.changes \
		$$HOME/@PACKAGE@/

pac-hare: sysconfig-$(VERSION).tar.bz2
	mkdir -p $$HOME/Devel/cvstree/checkouts/@PACKAGE@
	cp -p @PACKAGE@-$(VERSION).tar.bz2 \
		package/sysconfig.spec \
		package/sysconfig.changes \
		$$HOME/Devel/cvstree/checkouts/@PACKAGE@/

usrsrcsysconfig: sysconfig-$(VERSION).tar.bz2
	rm -rf /usr/src/sysconfig; \
	mkdir /usr/src/sysconfig; \
	cp -p @PACKAGE@-$(VERSION).tar.bz2 \
		package/sysconfig.spec \
		package/sysconfig.changes \
		/usr/src/@PACKAGE@/

tmppack: sysconfig-$(VERSION).tar.bz2
	rm -rf /tmp/@PACKAGE@-$(VERSION); \
	mkdir /tmp/@PACKAGE@-$(VERSION); \
	cp -p @PACKAGE@-$(VERSION).tar.bz2 \
		package/sysconfig.spec \
		package/sysconfig.changes \
		/tmp/@PACKAGE@-$(VERSION)/

cl:
	svn log . -v > $(distdir)/ChangeLog

