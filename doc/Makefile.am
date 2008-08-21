## Process this file with automake to produce Makefile.in

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

man_MANS = 	ifup.8 \
		ifdown.8 \
		ifservices.5 \
		ifstatus.8 \
		ifcfg.5 \
		ifcfg-bonding.5 \
		ifcfg-bridge.5 \
		ifcfg-tunnel.5 \
		ifcfg-vlan.5 \
		ifcfg-wireless.5 \
		hwup.8 \
		hwdown.8 \
		hwstatus.8 \
		getcfg.8 \
		getcfg-all-info.8 \
		getcfg-device.8 \
		getcfg-interface.8 \
		getcfg-make-hwdesc.8 \
		getcfg-parse-hwdesc.8 \
		modify_resolvconf.8 \
		netconfig.8 \
		routes.5

doc_DATA =	README COPYING Contents Network README.s390 \
		README.storage \
		README.netconfig netconfig.odg netconfig.png

sysconfig_hardware_DATA = README.hwcfg_and_device_initialisation

EXTRA_DIST = $(doc_DATA) $(man_MANS) $(sysconfig_hardware_DATA)
