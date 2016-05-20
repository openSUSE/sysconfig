## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = hardware netconfig.d ppp

sysconfig_network_scripts_SCRIPTS =	functions.rpm-utils \
					functions.netconfig

sbin_SCRIPTS =	netconfig

EXTRA_DIST = 	$(sysconfig_network_scripts_SCRIPTS) \
		$(sbin_SCRIPTS)

check: 
	list='$(sysconfig_network_scripts_SCRIPTS) $(sbin_SCRIPTS)' ; \
	for i in $$list; do bash -n $(srcdir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done
