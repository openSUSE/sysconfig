## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES =  $(BUILT_SOURCES)
EXTRA_DIST =	$(templates) \
		$(sysconfig_network_DATA) \
		$(fillup_templates_allstatic)
BUILT_SOURCES = $(fillup_templates_generated)

SUBDIRS = hardware

sysconfig_network_DATA =	ifcfg.template

fillup_templates_allstatic  =	sysconfig.dhcp-network
fillup_templates_generated  =	sysconfig.config-network

fillup_templates_generated +=	sysconfig.config-netconfig
netconfig_modules =
if netconfig_dns
fillup_templates_generated +=	sysconfig.config-netconfig-dns
netconfig_modules          +=	dns-resolver dns-bind dns-dnsmasq
endif
if netconfig_nis
fillup_templates_generated +=	sysconfig.config-netconfig-nis
netconfig_modules          +=	nis
endif
if netconfig_ntp
fillup_templates_generated +=	sysconfig.config-netconfig-ntp
netconfig_modules          +=	ntp-runtime
endif

fillup_templates_DATA =	$(fillup_templates_allstatic) \
			$(fillup_templates_generated)

templates = $(fillup_templates_generated:=.in)

substitute_vars = $(AM_V_GEN)sed \
	-e "s|[@]libexec_netconfigdir[@]|$(libexec_netconfigdir)|g"	\
	-e "s|[@]netconfig_modules[@]|$(strip $(netconfig_modules))|g"

%: %.in $(top_builddir)/config.status
	@echo "substituting variables in $< > $@"
	@rm -f $@ $@.tmp
	@$(substitute_vars) < $< > $@.tmp && mv $@.tmp $@

