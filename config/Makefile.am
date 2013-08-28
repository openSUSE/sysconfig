## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = hardware

sysconfig_network_DATA =	ifcfg.template \
				ifcfg-lo \
				ifroute-lo

udevrules_DATA = 77-network.rules

fillup_templates_DATA =	sysconfig.dhcp-network \
			sysconfig.config-network

omc_svcinfo_DATA = network.xml

unit_DATA = network.service network@.service

EXTRA_DIST = $(sysconfig_network_DATA) \
	     $(udevrules_DATA) \
	     $(fillup_templates_DATA) \
	     $(omc_svcinfo_DATA) \
	     $(unit_DATA)

install-data-hook:
	mkdir -p  $(DESTDIR)@sysconfig_networkdir@/providers
	chmod 700 $(DESTDIR)@sysconfig_networkdir@/providers
	mkdir -p  $(DESTDIR)@sysconfig_networkdir@/if-up.d
	mkdir -p  $(DESTDIR)@sysconfig_networkdir@/if-down.d
	chmod 755 $(DESTDIR)@sysconfig_networkdir@/if-up.d
	chmod 755 $(DESTDIR)@sysconfig_networkdir@/if-down.d
	mkdir -p  $(DESTDIR)@pppdir@/ip-up.d
	mkdir -p  $(DESTDIR)@pppdir@/ip-down.d
	chmod 755 $(DESTDIR)@pppdir@/ip-up.d
	chmod 755 $(DESTDIR)@pppdir@/ip-down.d
	mkdir -p  $(DESTDIR)@sysconfig_networkdir@/ifservices.template
	ln -s ../../../init.d/autofs  $(DESTDIR)@sysconfig_networkdir@/ifservices.template/K60autofs 
	ln -s ../../../init.d/ypbind  $(DESTDIR)@sysconfig_networkdir@/ifservices.template/K70ypbind 
	ln -s ../../../init.d/rpcbind $(DESTDIR)@sysconfig_networkdir@/ifservices.template/S10rpcbind 
	ln -s ../../../init.d/ypbind  $(DESTDIR)@sysconfig_networkdir@/ifservices.template/S30ypbind 
	ln -s ../../../init.d/autofs  $(DESTDIR)@sysconfig_networkdir@/ifservices.template/S40autofs 
	chmod 644  $(DESTDIR)@omc_svcinfodir@/network.xml

