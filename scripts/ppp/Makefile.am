## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

ppp_SCRIPTS	= ip-up netconfig

EXTRA_DIST = $(ppp_SCRIPTS)

install-data-hook:
	install -d -m 0755  $(DESTDIR)$(pppdir)/ip-up.d
	install -d -m 0755  $(DESTDIR)$(pppdir)/ip-down.d
	install -d -m 0755  $(DESTDIR)$(pppdir)/ipv6-up.d
	install -d -m 0755  $(DESTDIR)$(pppdir)/ipv6-down.d
	install -d -m 0755  $(DESTDIR)$(pppdir)/pre-start.d
	install -d -m 0755  $(DESTDIR)$(pppdir)/post-stop.d
	ln -sf ip-up        $(DESTDIR)$(pppdir)/ip-down
	ln -sf ip-up        $(DESTDIR)$(pppdir)/ipv6-up
	ln -sf ip-up        $(DESTDIR)$(pppdir)/ipv6-down
	ln -sf ip-up        $(DESTDIR)$(pppdir)/pre-start
	ln -sf ip-up        $(DESTDIR)$(pppdir)/post-stop
	ln -sf ../netconfig $(DESTDIR)$(pppdir)/ip-up.d/10-netconfig
	ln -sf ../netconfig $(DESTDIR)$(pppdir)/ip-down.d/90-netconfig
	ln -sf ../netconfig $(DESTDIR)$(pppdir)/pre-start.d/10-netconfig
	ln -sf ../netconfig $(DESTDIR)$(pppdir)/post-stop.d/90-netconfig

check:
	list='$(ppp_SCRIPTS)'; \
	for i in $$list; do bash -n $(srcdir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done

