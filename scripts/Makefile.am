## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = hardware netconfig.d

sysconfig_network_scripts_SCRIPTS =	functions \
					functions.common \
					functions.netconfig \
					functions.rpm-utils \
					convert_to_netconfig_dns \
					convert_to_netconfig_nis \
					ifdown-connection \
					ifup-802.1q \
					ifup-autoip \
					ifup-bridge \
					ifup-bonding \
					ifup-infiniband \
					ifup-ppp \
					ifup-route \
					ifup-services \
					ifup-skel \
					ifup-sysctl \
					ifup-wireless \
					ifup-tunnel \
					ifplugd-selectif \
					if-up.ndp-proxy \
					dhcpcd-hook \
					extradebug \
					link_wait \
					move_shm_sysconfig.sh \
					create-ifservices-directory

sbin_SCRIPTS =	ifup \
		ifup-dhcp \
		netconfig

init_d_SCRIPTS = network network-remotefs

ppp_SCRIPTS = 	ip-up \
		poll.tcpip

nm_dispatcher_SCRIPTS = netcontrol_services \
		netcontrol_global_hooks

EXTRA_DIST = 	$(sysconfig_network_scripts_SCRIPTS) \
		$(sbin_SCRIPTS) \
		$(init_d_SCRIPTS) \
		$(ppp_SCRIPTS) \
		$(nm_dispatcher_SCRIPTS)

install-data-hook:
	ln -sf ifup 			$(DESTDIR)/$(sbindir)/ifdown
	ln -sf ifup 			$(DESTDIR)/$(sbindir)/ifstatus
	ln -sf ifup 			$(DESTDIR)/$(sbindir)/ifprobe
	ln -sf ifup 			$(DESTDIR)/$(sbindir)/ifrenew
	ln -sf ifup-dhcp 		$(DESTDIR)/$(sbindir)/ifdown-dhcp
	ln -sf ifup-dhcp		$(DESTDIR)/$(sbindir)/ifstatus-dhcp
	ln -sf ifup-dhcp		$(DESTDIR)/$(sbindir)/ifrenew-dhcp
	ln -sf /etc/init.d/network 	$(DESTDIR)/$(sbindir)/rcnetwork
	ln -sf ifup-route 		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-route
	ln -sf ifup-route 		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-route
	ln -sf ifup-sysctl              $(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-sysctl
	ln -sf ifup-sysctl              $(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-sysctl
	ln -sf ifdown-connection	$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-connection
	ln -sf ifup-wireless		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-wireless
	ln -sf ifup-wireless		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-wireless
	ln -sf ifup-tunnel              $(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-tunnel
	ln -sf ifup-tunnel              $(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-tunnel
	ln -sf ifup-ppp			$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-ppp
	ln -sf ifup-ppp			$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-ppp
	ln -sf ifup-802.1q		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-802.1q
	ln -sf ifup-802.1q		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-802.1q
	ln -sf ifup-services		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-services
	ln -sf ifup-services		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-services
	ln -sf ifup-autoip		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-autoip
	ln -sf ifup-autoip		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-autoip
	ln -sf ifup-bridge		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-bridge
	ln -sf ifup-bridge		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-bridge
	ln -sf ifup-bonding		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-bonding
	ln -sf ifup-bonding		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-bonding
	ln -sf ifup-infiniband		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifdown-infiniband
	ln -sf ifup-infiniband		$(DESTDIR)/$(sysconfig_network_scriptsdir)/ifstatus-infiniband
	ln -sf ip-up			$(DESTDIR)/$(pppdir)/ip-down
	ln -sf ip-up			$(DESTDIR)/$(pppdir)/ipv6-up
	ln -sf ip-up			$(DESTDIR)/$(pppdir)/ipv6-down
	mkdir -p -m755			$(DESTDIR)$(sysconfig_networkdir)/if-up.d
	mkdir -p -m755			$(DESTDIR)$(sysconfig_networkdir)/if-down.d
	ln -sf ../scripts/if-up.ndp-proxy	$(DESTDIR)/$(sysconfig_networkdir)/if-up.d/ndp-proxy
	ln -sf ../scripts/if-up.ndp-proxy	$(DESTDIR)/$(sysconfig_networkdir)/if-down.d/ndp-proxy

check: 
	list='$(sysconfig_network_scripts_SCRIPTS) $(sbin_SCRIPTS) $(init_d_SCRIPTS) $(ppp_SCRIPTS) $(nm_dispatcher_SCRIPTS)'; \
	for i in $$list; do bash -n $(srcdir)/$$i && printf '%-30s %s\n' $$i "Syntax OK" || exit 1; done
