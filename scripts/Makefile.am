## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = hardware netconfig.d ppp

libexecdir_netconfig_scripts_SCRIPTS =	functions.rpm-utils \
					functions.netconfig

sbin_SCRIPTS =	netconfig

EXTRA_DIST = 	$(libexecdir_netconfig_scripts_SCRIPTS) \
		$(sbin_SCRIPTS)

check: 
	list='$(libexecdir_netconfig_scripts_SCRIPTS) $(sbin_SCRIPTS)' ; \
	for i in $$list; do bash -n $(srcdir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done
