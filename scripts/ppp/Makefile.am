## Process this file with automake to produce Makefile.in

CLEANFILES		= *~
DISTCLEANFILES		= $(BUILT_SOURCES)
MAINTAINERCLEANFILES	= Makefile.in
BUILT_SOURCES		= $(libexec_ppp_generated)
EXTRA_DIST		= $(libexec_ppp_templates)

libexec_ppp_generated	= ip-up
libexec_ppp_SCRIPTS	= $(libexec_ppp_generated)
libexec_ppp_templates	= $(libexec_ppp_generated:=.in)

substitute_vars = $(AM_V_GEN)sed \
	-e "s|[@]sysconf_pppdir[@]|$(sysconf_pppdir)|g"	\
	-e "s|[@]libexec_pppdir[@]|$(libexec_pppdir)|g"

%: %.in $(top_builddir)/config.status
	@echo "substituting variables in $< > $@"
	@rm -f $@ $@.tmp
	@$(substitute_vars) < $< > $@.tmp && mv $@.tmp $@

install-data-hook:
	$(MKDIR_P)	$(DESTDIR)$(sysconf_pppdir)
	for n in ip-up ip-down ipv6-up ipv6-down ip-pre-up pre-start post-stop		; do \
		$(MKDIR_P) $(DESTDIR)$(libexec_pppdir)/$$n.d				; \
		$(MKDIR_P) $(DESTDIR)$(sysconf_pppdir)/$$n.d				; \
		$(LN_S) -f $(libexec_pppdir)/ip-up $(DESTDIR)$(sysconf_pppdir)/$$n	; \
	done

check-local:
	list='$(libexec_ppp_SCRIPTS)'; \
	for i in $$list; do bash -n $(builddir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done

