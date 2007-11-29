## Process this file with automake to produce Makefile.in

SUBDIRS = hardware

sysconfig_network_DATA =	ifcfg.template \
				ifcfg-lo \
				ifroute-lo

udevrules_DATA = 77-network.rules

fillup_templates_DATA =	remove_old_netconfig \
			sysconfig.dhcp-network \
			sysconfig.config-network

omc_svcinfo_DATA = network.xml

EXTRA_DIST = $(sysconfig_network_DATA) \
	     $(udevrules_DATA) \
	     $(fillup_templates_DATA) \
	     $(omc_svcinfo_DATA)

install-data-hook:
	mkdir -p  $(DESTDIR)$(prefix)@sysconfig_networkdir@/providers
	chmod 700 $(DESTDIR)$(prefix)@sysconfig_networkdir@/providers
	mkdir -p  $(DESTDIR)$(prefix)@sysconfig_networkdir@/if-up.d
	mkdir -p  $(DESTDIR)$(prefix)@sysconfig_networkdir@/if-down.d
	chmod 755 $(DESTDIR)$(prefix)@sysconfig_networkdir@/if-up.d
	chmod 755 $(DESTDIR)$(prefix)@sysconfig_networkdir@/if-down.d
	mkdir -p  $(DESTDIR)$(prefix)@pppdir@/ip-up.d
	mkdir -p  $(DESTDIR)$(prefix)@pppdir@/ip-down.d
	chmod 755  $(DESTDIR)$(prefix)@pppdir@/ip-up.d
	chmod 755  $(DESTDIR)$(prefix)@pppdir@/ip-down.d
	mkdir -p  $(DESTDIR)$(prefix)@sysconfig_networkdir@/ifservices.template
	ln -s ../../../init.d/autofs $(DESTDIR)$(prefix)@sysconfig_networkdir@/ifservices.template/K60autofs 
	ln -s ../../../init.d/ypbind $(DESTDIR)$(prefix)@sysconfig_networkdir@/ifservices.template/K70ypbind 
	ln -s ../../../init.d/portmap $(DESTDIR)$(prefix)@sysconfig_networkdir@/ifservices.template/S10portmap 
	ln -s ../../../init.d/ypbind $(DESTDIR)$(prefix)@sysconfig_networkdir@/ifservices.template/S30ypbind 
	ln -s ../../../init.d/autofs $(DESTDIR)$(prefix)@sysconfig_networkdir@/ifservices.template/S40autofs 
	chmod 644  $(DESTDIR)$(prefix)@omc_svcinfodir@/network.xml
