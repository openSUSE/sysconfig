## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = modprobe.d

sysconfig_network_DATA =	ifcfg.template

fillup_templates_DATA =	sysconfig.dhcp-network \
			sysconfig.config-network

EXTRA_DIST = $(sysconfig_network_DATA) \
	     $(fillup_templates_DATA)
