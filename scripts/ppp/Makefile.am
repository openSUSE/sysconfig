## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

install-data-hook:
	install -d -m 0755  $(DESTDIR)$(pppdir)/ip-up.d
	install -d -m 0755  $(DESTDIR)$(pppdir)/ip-down.d
	install -d -m 0755  $(DESTDIR)$(pppdir)/ipv6-up.d
	install -d -m 0755  $(DESTDIR)$(pppdir)/ipv6-down.d
	install -d -m 0755  $(DESTDIR)$(pppdir)/pre-start.d
	install -d -m 0755  $(DESTDIR)$(pppdir)/post-stop.d

