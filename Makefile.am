## Process this file with automake to produce Makefile.in

EXTRA_DIST = 	VERSION \
		sysconfig.spec \
		sysconfig.spec.in 

AUTOMAKE_OPTIONS = foreign dist-bzip2 no-dist-gzip
CONFIGURE_DEPENDENCIES = VERSION

CLEANFILES = *~
MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub  \
	configure depcomp install-sh ltmain.sh Makefile.in \
	missing sysconfig.spec package/sysconfig.spec      \
	$(DIST_ARCHIVES) $(PACKAGE)-$(VERSION).tar.bz2     \
	package/$(PACKAGE)-$(VERSION).tar.bz2

SUBDIRS = doc scripts tools config

$(PACKAGE)-$(VERSION).tar.bz2: all check dist distcheck

package/sysconfig.spec: sysconfig.spec
	@rm -f package/sysconfig.spec
	@cp -vp sysconfig.spec package/

package/$(PACKAGE)-$(VERSION).tar.bz2: $(PACKAGE)-$(VERSION).tar.bz2
	@rm -f package/$(PACKAGE)-$(VERSION).tar.bz2
	@cp -vp $(PACKAGE)-$(VERSION).tar.bz2 package/

package package-local:  package/$(PACKAGE)-$(VERSION).tar.bz2 \
			package/sysconfig.spec show-commit-state
	@echo "=========================================================================="
	@ls -l package/
	@echo "=========================================================================="

dist-hook: VERSION sysconfig.spec $(distdir)/ChangeLog

$(distdir)/ChangeLog:
	@if  [ -d $(top_srcdir)/.git ]; then \
	  if ! git --version &>/dev/null ; then \
	    echo 'ERROR: Unable to find the '\'git\'' version control client!'; \
	    echo '       Please install git (git-core) and try again.'        ; \
	    exit 1 ; \
	  fi >&2 ; \
	  LANG=C git log --decorate=short -- $(top_srcdir) > $(distdir)/ChangeLog ; \
	elif [ -d $(top_srcdir)/.svn ]; then \
	  if ! svn --version &>/dev/null ; then \
	    echo 'ERROR: Unable to find the '\'svn\'' subversion client!' ; \
	    echo '       Please install subversion and try again.'        ; \
	    exit 1 ; \
	  fi >&2 ; \
	  LANG=C svn log $(top_srcdir) -v > $(distdir)/ChangeLog ; \
	fi

show-commit-state:
	@if  [ -d $(top_srcdir)/.git ]; then \
	  if ! git --version &>/dev/null ; then \
	    echo 'ERROR: Unable to find the '\'git\'' version control client!'; \
	    echo '       Please install git (git-core) and try again.'        ; \
	    exit 1 ; \
	  fi >&2 ; \
	  cd $(top_srcdir) && \
	  LC_ALL=C git status --all --quiet 2>&1 | \
	  grep -qsE 'Untracked|to be committed' && { \
	    echo -e "##\n## *** NOTE: There are uncommitted changes:\n##" ; \
	    git status --quiet 2>&1 || : ; \
	  } || : ; \
	elif [ -d $(top_srcdir)/.svn ]; then \
	  if ! svn   --version &>/dev/null ; then \
	    echo 'ERROR: Unable to find the '\'svn\'' subversion client!' ; \
	    echo '       Please install subversion and try again.'        ; \
	    exit 1 ; \
	  fi >&2 ; \
	  cd $(top_srcdir) && \
	    out=`LC_ALL=C svn status --show-updates --quiet 2>&1` ; \
	  if [ "x`echo \"$$out\" | grep -v '^Status against revision'`" != x ] ; then \
	    echo -e "##\n## *** NOTE: There are uncommitted changes:\n##" ; \
	    echo "$$out" ; \
	  fi ; \
	fi

### ====================================================
pac-poeml: package
	mkdir -p $$HOME/pac/$(PACKAGE)
	cp -p   package/$(PACKAGE)-$(VERSION).tar.bz2 \
		package/sysconfig.spec \
		package/sysconfig.changes \
		$$HOME/pac/$(PACKAGE)/

pac-zoz: package
	mkdir -p $$HOME/$(PACKAGE)
	cp -p   package/$(PACKAGE)-$(VERSION).tar.bz2 \
		package/sysconfig.spec \
		package/sysconfig.changes \
		$$HOME/$(PACKAGE)/

pac-hare: package
	mkdir -p $$HOME/Devel/cvstree/checkouts/$(PACKAGE)
	cp -p   package/$(PACKAGE)-$(VERSION).tar.bz2 \
		package/sysconfig.spec \
		package/sysconfig.changes \
		$$HOME/Devel/cvstree/checkouts/$(PACKAGE)/

pac-mt: package
	@if test -d "/space/mt/osc/home:mtomaschewski:Factory/sysconfig" ; then \
		cp -vp package/$(PACKAGE)-$(VERSION).tar.bz2 \
		       package/sysconfig.spec \
		       package/sysconfig.changes \
		      "/space/mt/osc/home:mtomaschewski:Factory/sysconfig/" ; \
	fi

usrsrcsysconfig: package
	rm -rf /usr/src/sysconfig; \
	mkdir /usr/src/sysconfig; \
	cp -p   package/$(PACKAGE)-$(VERSION).tar.bz2 \
		package/sysconfig.spec \
		package/sysconfig.changes \
		/usr/src/$(PACKAGE)/

tmppack: package
	rm -rf /tmp/$(PACKAGE)-$(VERSION); \
	mkdir /tmp/$(PACKAGE)-$(VERSION); \
	cp -p   package/$(PACKAGE)-$(VERSION).tar.bz2 \
		package/sysconfig.spec \
		package/sysconfig.changes \
		/tmp/$(PACKAGE)-$(VERSION)/

cl: $(distdir)/ChangeLog

