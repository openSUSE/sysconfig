## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = hardware

sysconfig_network_DATA =	ifcfg.template

fillup_templates_DATA =	sysconfig.dhcp-network \
			sysconfig.config-network

omc_svcinfo_DATA = network.xml

EXTRA_DIST = $(sysconfig_network_DATA) \
	     $(fillup_templates_DATA) \
	     $(omc_svcinfo_DATA)

install-data-hook:
	chmod 644  $(DESTDIR)@omc_svcinfodir@/network.xml

