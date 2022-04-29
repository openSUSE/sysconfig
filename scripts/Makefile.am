## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES =  $(BUILT_SOURCES)
EXTRA_DIST =	$(templates) \
		$(libexec_netconfig_DATA) \
		$(libexec_netconfig_ppp_SCRIPTS)
BUILT_SOURCES = $(sbin_SCRIPTS)

SUBDIRS = hardware netconfig.d ppp

libexec_netconfig_DATA		= functions.netconfig
sysconfig_network_scriptsdir	= $(sysconfig_networkdir)/scripts

sbin_generated			= netconfig
sbin_SCRIPTS			= $(sbin_generated)

libexec_netconfig_pppdir	= $(libexec_netconfigdir)/ppp
libexec_netconfig_ppp_SCRIPTS	= ip-up

templates			= $(sbin_generated:=.in)

substitute_vars	= $(AM_V_GEN)sed \
	-e "s|[@]libexec_netconfigdir[@]|$(libexec_netconfigdir)|g"

%: %.in $(top_builddir)/config.status
	@echo "substituting variables in $< > $@"
	@rm -f $@ $@.tmp
	@$(substitute_vars) < $< > $@.tmp && mv $@.tmp $@

install-data-hook:
	$(MKDIR_P) $(DESTDIR)$(sysconfig_network_scriptsdir)
	$(MKDIR_P) $(DESTDIR)$(pppdir)/ip-up.d
	$(MKDIR_P) $(DESTDIR)$(pppdir)/ip-down.d
	$(MKDIR_P) $(DESTDIR)$(pppdir)/pre-start.d
	$(MKDIR_P) $(DESTDIR)$(pppdir)/post-stop.d
	$(LN_S) -f $(libexec_netconfigdir)/functions.netconfig $(DESTDIR)$(sysconfig_network_scriptsdir)
	$(LN_S) -f $(libexec_netconfig_pppdir)/ip-up $(DESTDIR)$(pppdir)/ip-up.d/10-netconfig
	$(LN_S) -f $(libexec_netconfig_pppdir)/ip-up $(DESTDIR)$(pppdir)/ip-down.d/90-netconfig
	$(LN_S) -f $(libexec_netconfig_pppdir)/ip-up $(DESTDIR)$(pppdir)/pre-start.d/10-netconfig
	$(LN_S) -f $(libexec_netconfig_pppdir)/ip-up $(DESTDIR)$(pppdir)/post-stop.d/90-netconfig

check-local:
	@for i in $(sbin_SCRIPTS) ; do bash -n $(builddir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done
	@for i in $(libexec_netconfig_DATA) ; do bash -n $(srcdir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done
	@for i in $(libexec_netconfig_ppp_SCRIPTS) ; do bash -n $(srcdir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done
