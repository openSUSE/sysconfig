## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES =  $(BUILT_SOURCES)
EXTRA_DIST = $(templates)
BUILT_SOURCES = $(libexec_netconfig_d_scripts_generated)

libexec_netconfig_d_scripts_generated	= dns-bind dns-resolver dns-dnsmasq nis ntp-runtime 
libexec_netconfig_d_scripts_SCRIPTS	= $(libexec_netconfig_d_scripts_generated)

templates = $(libexec_netconfig_d_scripts_generated:=.in)

substitute_vars = $(AM_V_GEN)sed \
	-e "s|[@]libexec_netconfigdir[@]|$(libexec_netconfigdir)|g"

%: %.in $(top_builddir)/config.status
	@echo "substituting variables in $< > $@"
	@rm -f $@ $@.tmp
	@$(substitute_vars) < $< > $@.tmp && mv $@.tmp $@

check-local:
	@for i in $(libexec_netconfig_d_scripts_SCRIPTS) ; \
	do bash -n $(builddir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done

