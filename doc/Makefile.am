## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES =  $(BUILT_SOURCES)
EXTRA_DIST = $(templates) $(doc_DATA)
BUILT_SOURCES = $(mans_generated)

mans_generated = netconfig.8

man_MANS =	$(mans_generated)

doc_DATA =	COPYING Contents \
		netconfig.png

templates = $(mans_generated:=.in)

substitute_vars = $(AM_V_GEN)sed \
	-e "s|[@]libexec_netconfigdir[@]|$(libexec_netconfigdir)|g"

%: %.in $(top_builddir)/config.status
	@echo "substituting variables in $< > $@"
	@rm -f $@ $@.tmp
	@$(substitute_vars) < $< > $@.tmp && mv $@.tmp $@

