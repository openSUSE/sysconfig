## Process this file with automake to produce Makefile.in

sbin_SCRIPTS =	hwup

sysconfig_hardware_SCRIPTS = hwcfg2rules

EXTRA_DIST = $(sbin_SCRIPTS) $(sysconfig_hardware_SCRIPTS)

install-data-hook:
	ln -sf hwup          $(DESTDIR)/$(sbindir)/hwdown
	ln -sf hwup          $(DESTDIR)/$(sbindir)/hwstatus

check: 
	list='$(sbin_SCRIPTS) $(sysconfig_hardware_SCRIPTS)'; \
	for i in $$list; do bash -n $(srcdir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done
