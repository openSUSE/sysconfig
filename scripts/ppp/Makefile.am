## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = $(ppp_SCRIPTS)

ppp_SCRIPTS	= ip-up

install-data-hook:
	$(MKDIR_P)       $(DESTDIR)$(pppdir)/ip-up.d
	$(MKDIR_P)       $(DESTDIR)$(pppdir)/ip-down.d
	$(MKDIR_P)       $(DESTDIR)$(pppdir)/ipv6-up.d
	$(MKDIR_P)       $(DESTDIR)$(pppdir)/ipv6-down.d
	$(MKDIR_P)       $(DESTDIR)$(pppdir)/pre-start.d
	$(MKDIR_P)       $(DESTDIR)$(pppdir)/post-stop.d
	$(LN_S) -f ip-up $(DESTDIR)$(pppdir)/ip-down
	$(LN_S) -f ip-up $(DESTDIR)$(pppdir)/ipv6-up
	$(LN_S) -f ip-up $(DESTDIR)$(pppdir)/ipv6-down
	$(LN_S) -f ip-up $(DESTDIR)$(pppdir)/pre-start
	$(LN_S) -f ip-up $(DESTDIR)$(pppdir)/post-stop

check:
	list='$(ppp_SCRIPTS)'; \
	for i in $$list; do bash -n $(srcdir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done

