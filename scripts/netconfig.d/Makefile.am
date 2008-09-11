## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

netconfig_d_scripts_SCRIPTS = dns-bind dns-resolver dns-dnsmasq nis ntp-runtime 

EXTRA_DIST = 	$(netconfig_d_scripts_SCRIPTS)

check: 
	list='$(netconfig_d_scripts_SCRIPTS)'; \
	for i in $$list; do bash -n $(srcdir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done

