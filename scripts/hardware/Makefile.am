## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

sysconfig_hardware_SCRIPTS = hwcfg2rules

EXTRA_DIST = $(sysconfig_hardware_SCRIPTS)

check: 
	list='$(sysconfig_hardware_SCRIPTS)'; \
	for i in $$list; do bash -n $(srcdir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done
