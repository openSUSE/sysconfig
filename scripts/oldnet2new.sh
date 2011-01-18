#!/bin/bash
#
# oldnet2new.sh
#
# Copyright (c) 2001, 2002 SuSE GmbH Nuernberg, Germany.  All rights reserved.
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA 02111-1307 USA
#
# Author: Mads Martin Jørgensen <mmj@suse.de>
#         Arvin Schnell <arvin@suse.de>
#
# This script generates the new 8.0 style network config files
# from the old style in /etc/rc.config and others.
#

if test -n "$YAST_IS_RUNNING" ; then
	INSSERV="sbin/insserv -f"
else
	INSSERV="sbin/insserv"
fi

# source rc.config if it exists
if test -f etc/rc.config; then
	source etc/rc.config
else
	# nothing to do! Feierabend!
	exit 0
fi

test -d etc/sysconfig/network || mkdir -p etc/sysconfig/network

CFGFILEPREFIX="etc/sysconfig/network/"

# good place to convert obsolete iSeries device names... veth --> eth
for i in _0 _1 _2 _3; do
	eval NETDEV=\$NETDEV$i
	case "$NETDEV" in
		veth*)  eval NETDEV$i=\${NETDEV#v};;
	esac
done; unset NETDEV


#
# go through all network devices configured in rc.config:
#
# 		 NETDEV_0="eth0"
# 		 NETDEV_1="eth0:1"
# 		 NETDEV_2="eth0:2"
# 		 NETDEV_3="eth0:3"
# 		 NETDEV_4="eth0:4"
# 		 NETDEV_5="eth0:5"


#
# create the filenames of device configurations
#
# NETDEV_10=eth0:10 will be converted to CFGFILE_10=etc/sysconfig/network/ifcfg-eth0:10
#
for i in ${!NETDEV_*}; do
	name=CFGFILE_${i#NETDEV_}
	eval NETDEV=\$$i
	value=${CFGFILEPREFIX}ifcfg-$NETDEV
	eval $name=$value
done; unset NETDEV

# convert all device config names for PCMCIA interfaces:
#
# etc/sysconfig/network/ifcfg-eth0:1
# would become
# etc/sysconfig/network/ifcfg-eth-pcmcia-0:1
#
# Note: IP aliases are supported ;-)
#
for i in ${!CFGFILE_*}; do
	name=CFGFILE_PCMCIA_${i#CFGFILE_}
	eval CFGFILE_PCMCIA=\$$i
	tmp1=${CFGFILE_PCMCIA%%[0-9]*}
	tmp2=${CFGFILE_PCMCIA##*[^0-9:]}
	CFGFILE_PCMCIA=$tmp1-pcmcia-$tmp2
	eval $name=$CFGFILE_PCMCIA
done; unset CFGFILE_PCMCIA


#
# get NETMASK_xyz and BROADCAST_xyz for all interfaces
#
# IFCONFIG_16="192.168.5.245 broadcast 192.168.5.255 netmask 255.255.255.0"
#
function get_ifconfig() {
	shift # IP address comes first
	while [ $1 ]; do
		case $1 in
			netmask)	NETMASK=$2; shift;;
			broadcast)      BROADCAST=$2; shift;;
			pointopoint)    REMOTE_IPADDR=$2; shift;;
			mtu)      	MTU=$2; shift;;
		esac
		shift
	done
}
for i in ${!IFCONFIG_*}; do
	eval get_ifconfig \$$i
	eval       NETMASK_${i#IFCONFIG_}=$NETMASK; unset NETMASK
	eval     BROADCAST_${i#IFCONFIG_}=$BROADCAST; unset BROADCAST
	eval REMOTE_IPADDR_${i#IFCONFIG_}=$REMOTE_IPADDR; unset REMOTE_IPADDR
	eval           MTU_${i#IFCONFIG_}=$MTU; unset MTU
done



#
# set default STARTMODE
#
for i in ${!NETDEV_*}; do
	eval STARTMODE_${i#NETDEV_}=manual
done

# Mark them hotplug if PCMCIA
for I in $NETCONFIG_PCMCIA ; do
	eval STARTMODE$I="hotplug";
done


#
# Check for devices that are "activated" in rc.config and mark them such
#
#		#
#		# Networking
#		#
#		# Number of network cards: "_0" for one, "_0 _1 _2 _3" for four cards
#		#
#		NETCONFIG="_0 _1 _2 _3 _4 _5 _6 _7"
#
for I in $NETCONFIG ; do
	eval STARTMODE$I="onboot";
done


parse_ifconfig () {
	eval IFCFG=\$IFCONFIG$I
	DYNAMICIP="no"
	XXX=0
	NETINDEX=$I
	eval NETMASK$I=""
	for X in $IFCFG ; do
		case $XXX in
			0) ;;
			1) eval NETMASK$I=\$X
			   XXX=0 ;;
			2) PTPADDR=$X
			   XXX=0 ;;
		esac
		test "$X" = "netmask" && XXX=1
		test "$X" = "pointopoint" && XXX=2
		test "$X" = "dynamic" && DYNAMICIP="yes"
	done
	eval BROADCAST$I=""
}

for i in ${!NETDEV_*}; do
	I=_${i#NETDEV_}
	eval NETDEV=\$NETDEV$I;
	case "$NETDEV" in
		ippp*|isdn*)
			eval CFGFILE$I=$CFGFILEPREFIX"tmpcfg-\$NETDEV"
			parse_ifconfig;;
		*)	;;
	esac

	eval        IPADDR=\$IPADDR$I;
	eval     BROADCAST=\$BROADCAST$I;
	eval       NETMASK=\$NETMASK$I;
	eval REMOTE_IPADDR=\$REMOTE_IPADDR$I;
	eval           MTU=\$MTU$I;
	eval  LINK_OPTIONS=\$LINK_OPTIONS$I;

	if [ $NETDEV ] && [ "$BROADCAST" != "netmask" ] ; then
		eval CFGFILE=\$CFGFILE$I;
		eval STARTMODE=\$STARTMODE$I;
		if [ "$STARTMODE" = "hotplug" ]; then
			eval CFGFILE=\$CFGFILE_PCMCIA$I;
		fi;

		# Check if file already exists, if it does then append .rpmnew
		if [ -f $CFGFILE ] ; then
			CFGFILE=$CFGFILE".rpmnew";
			echo "Config file exists.";
			echo "Using $CFGFILE";
			> $CFGFILE
		else
			echo "Creating $CFGFILE";
		fi;

		# Handle the bootprotocol
		eval IFCONFIG=\$IFCONFIG$I;
		case "$IFCONFIG" in
			bootp)
				echo "BOOTPROTO=dhcp" >> $CFGFILE;
				;;
			dhcpclient)
				echo "BOOTPROTO=dhcp" >> $CFGFILE;
				;;
			*)
				echo "BOOTPROTO=none" >> $CFGFILE;
				;;
		esac

		# The IP address
		if [ "$IPADDR" ] ; then
			echo "IPADDR=$IPADDR" >> $CFGFILE;
		fi

		# The netmask
		[ "$NETMASK" ] && echo "NETMASK=$NETMASK" >> $CFGFILE;

		# The broadcast
		[ "$BROADCAST" ] && echo "BROADCAST=$BROADCAST" >> $CFGFILE;

		# The pointopoint address
		[ "$REMOTE_IPADDR" ] && echo "REMOTE_IPADDR=$REMOTE_IPADDR" >> $CFGFILE;

		# MTU
		[ "$MTU" ] && echo "MTU=$MTU" >> $CFGFILE;

		# The Link options linke media type, tunnel, ...
		[ "$LINK_OPTIONS" ] && echo "LINK_OPTIONS=$LINK_OPTIONS" >> $CFGFILE;

		# The boot mode
		echo "STARTMODE=$STARTMODE" >> $CFGFILE;

		# We need this for pcmcia
		if [ "$STARTMODE" = "hotplug" ]; then
			echo "DHCLIENT_SET_DOWN_LINK=yes" >> $CFGFILE;
		fi;

		case "$NETDEV" in
        	        ippp*)
				echo "PROTOCOL=syncppp"  >> $CFGFILE;
				echo "DYNAMICIP=$DYNAMICIP" >> $CFGFILE;
				echo "PTPADDR=$PTPADDR" >> $CFGFILE;
				echo "NETINDEX=$NETINDEX" >> $CFGFILE;
				;;
        	        isdn*)
				echo "PROTOCOL=rawip"  >> $CFGFILE;
				echo "PTPADDR=$PTPADDR" >> $CFGFILE;
				echo "NETINDEX=$NETINDEX" >> $CFGFILE;
				;;
			*) ;;
		esac
		case "$NETDEV" in
			ctc*|escon*)
				echo "WAIT_FOR_CONNECTION=30" >> $CFGFILE;;
		esac
	fi
done

MAINCONFFILE=$CFGFILEPREFIX/config

# Handle MODIFY_RESOLV_CONF_DYNAMICALLY
if [ "$MODIFY_RESOLV_CONF_DYNAMICALLY" ]; then
	sed -e \
	"s/^MODIFY_RESOLV_CONF_DYNAMICALLY=.*/MODIFY_RESOLV_CONF_DYNAMICALLY=$MODIFY_RESOLV_CONF_DYNAMICALLY/" \
	$MAINCONFFILE > "$MAINCONFFILE.new"
	if [ -s $MAINCONFFILE.new ]; then
		mv $MAINCONFFILE.new $MAINCONFFILE
	fi;
fi;

if [ "$CLOSE_CONNECTIONS" ]; then
	sed -e \
	"s/^CONNECTION_CLOSE_BEFORE_IFDOWN=.*/CONNECTION_CLOSE_BEFORE_IFDOWN=$CLOSE_CONNECTIONS/" \
	$MAINCONFFILE > "$MAINCONFFILE.new"
	if [ -s $MAINCONFFILE.new ]; then
		mv $MAINCONFFILE.new $MAINCONFFILE
	fi;
fi;


# Handle the dummy device--taken from the old /etc/init.d/dummy script
if [ "$NETCONFIG" != "YAST_ASK" -a "$SETUPDUMMYDEV" = "yes" ] ; then
	dummy=dummy0;
	while read  i j ;
	    do test $i = "dummy:" && dummy=dummy;
	done < /proc/net/dev
	ethdev=""
	defdev=""
	ipdef=""
	ipaddr=""
	for dev in $NETCONFIG; do
	    eval ndev=\$NETDEV$dev
	    if test -z "$defdev" ; then
		eval defdev=\$IPADDR$dev
		ipdef=$defdev
	    fi
	    case "$ndev" in
		eth*|tr*)
		    if test -z "$ethdev" ; then
			eval ethdev=\$IFCONFIG$dev
			eval ipeth=\$IPADDR$dev
		    fi
		    ;;
	    esac
	done
	if test -n "$ethdev" ; then
	    dev=$ethdev
	    ipaddr=$ipeth
	else
	    dev=$defdev
	    ipaddr=$ipdef
	fi

	if [ "$dev" -a "$dev" != "dhcpclient" ] ; then
		CFGFILE=$CFGFILEPREFIX"ifcfg-$dummy"

		echo "BOOTPROTO=none" > $CFGFILE
		echo "IPADR=$ipaddr" > $CFGFILE
		echo "NETMASK=255.255.255.255" > $CFGFILE
		echo "BROADCAST=255.255.255.255" > $CFGFILE
		echo "STARTMODE=onboot" > $CFGFILE
	fi
fi;


#Handle the hostname
FQHOSTNAME=$FQHOSTNAME""
if [ "$FQHOSTNAME" ] ; then
	echo "$FQHOSTNAME" > etc/HOSTNAME
fi


#Move /etc/route.conf and notify root to be aware of it
if [ -f etc/route.conf ] ; then
	mv etc/route.conf etc/sysconfig/network/routes
	echo "\
Hi, (Deutsche Übersetzung siehe unten)
As a part of the configuration changes made for 8.0 (/etc/sysconfig)
the routing configuration have moved from /etc/route.conf to
/etc/sysconfig/network/routes. Since we also now use the 'ip' com-
mand to setup the routing options etc. have now changed. So if you en-
counter problems with routing please consult the routing documentation
by doing 'man route.conf' and the documentation for the iproute2 pack-
age which can be found in /usr/share/doc/packages/iproute2.
Sorry for any inconvienience.

Bedingt durch die Konfigurationsumstellungen für SuSE Linux 8.0
(/etc/sysconfig) ist die Routing Konfiguration von /etc/route.conf
nach /etc/sysconfig/network/routes umgezogen. Da nun für die
Einstellungen das Kommando 'ip' verwendet wird, haben sich auch
die Optionen geändert. Sollten also Probleme mit dem Routing
auftreten, lesen Sie bitte die Dokumentation per 'man route.conf'
und die Dokumentation zum Paket iproute2 die unter
/usr/share/doc/packages/iproute2 zu finden ist.
" > var/adm/notify/messages/routing_config
fi

# Put the Unique number in the config file

UNIQUEINFFILE="var/lib/YaST/unique.inf"
if [ -f $UNIQUEINFFILE ]; then
while read inline; do
	case $inline in
		\[network\]*)
			while read netconf; do
				case $netconf in
					\[*\]*)
						break
						;;
					*)
						UNIQUESTR=`echo $netconf | gawk '{print $1}'`
						CONF=`echo $netconf | gawk '{print $2}'`
						CONFNUMBER=`echo $CONF | gawk -F';' '{print $2}'`
						if [ "$CONFNUMBER" = "" ]; then
							CONFNUMBER="0";
						fi;
						CONFFILE="etc/sysconfig/network/ifcfg-eth"$CONFNUMBER
						if [ -f $CONFFILE ]; then
							. $CONFFILE
							if [ ! $UNIQUE ]; then
								echo "UNIQUE='$UNIQUESTR'" >> $CONFFILE
							fi;
						fi;
						;;
				esac
			done
		;;
	esac
done < $UNIQUEINFFILE
fi;


# Remove stuff from rc.config
if [ -x bin/fillup ] ; then
	bin/fillup -q -t -r -i -d "=" etc/rc.config \
	var/adm/fillup-templates/remove_old_netconfig /dev/null
	test -s etc/rc.config.new && mv etc/rc.config.new etc/rc.config
fi
$INSSERV etc/init.d/network



#
# now turn attention to the dsl and modem config
#

function pppoed()
{
    if [ -r etc/pppoed.conf ] ; then
	sed -n -e "s/^$1[ ]*=[ ]*\(.*\)/\1/p" etc/pppoed.conf | \
	sed -n -e "s/\"*\([^\"]*\)\"*/\"\1\"/p"
    else
	sed -n -e "s/^$1[ ]*=[ ]*\(.*\)/\1/p" etc/pppoed.conf.rpmsave | \
	sed -n -e "s/\"*\([^\"]*\)\"*/\"\1\"/p"
    fi
}

function wvdial()
{
    sed -n -e "/\[Dialer $1/ {:x;p;n;/\[Dialer/q;bx}" etc/wvdial.conf | \
    sed -n -e "s/^$2[ ]*=[ ]*\(.*\)/\1/p" | \
    sed -n -e "s/\"*\([^\"]*\)\"*/\"\1\"/p"
}

function wvdial_bool()
{
    sed -n -e "/\[Dialer $1/ {:x;p;n;/\[Dialer/q;bx}" etc/wvdial.conf | \
    sed -n -e "s/^$2[ ]*=[ ]*\(.*\)/\1/p" | \
    sed -n -e "s/\"*\([^\"]*\)\"*/\"\1\"/p" | \
    sed -n -e "s/\"\(1\|yes\)\"/\"yes\"/p; t; s/.*/\"no\"/p"
}


# rc.dialout will be renamed later during the update
# since 8.1 this can also happen before this script runs

if ( [ -r etc/rc.dialout ] && `grep --quiet "^[^# ]" etc/rc.dialout` ) ||
   ( [ -r etc/rc.dialout.rpmsave ] && `grep --quiet "^[^# ]" etc/rc.dialout.rpmsave` ) ; then

    if [ -r etc/rc.dialout ] ; then
	. etc/rc.dialout
    else
	. etc/rc.dialout.rpmsave
    fi

    $INSSERV etc/init.d/smpppd

    if [ -n "$ADSL_NAME_0" ] && [ -r etc/pppoed.conf -o -r etc/pppoed.conf.rpmsave ] ; then
	if [ ! -r etc/sysconfig/network/ifcfg-dsl0 ] && [ ! -r etc/sysconfig/network/providers/dsl-provider0 ] ; then

	    echo "# generated during update" > etc/sysconfig/network/ifcfg-dsl0

	    echo PROVIDER=\"dsl-provider0\" >> etc/sysconfig/network/ifcfg-dsl0

	    echo STARTMODE=\"manual\" >> etc/sysconfig/network/ifcfg-dsl0
	    echo BOOTPROTO=\"none\" >> etc/sysconfig/network/ifcfg-dsl0

	    echo DEVICE=`pppoed "interface"` >> etc/sysconfig/network/ifcfg-dsl0
	    echo PPPMODE=\"pppoe\" >> etc/sysconfig/network/ifcfg-dsl0

	    echo "# generated during update" > etc/sysconfig/network/providers/dsl-provider0
	    chmod 0600 etc/sysconfig/network/providers/dsl-provider0

	    echo PROVIDER=\"$ADSL_NAME_0\" >> etc/sysconfig/network/providers/dsl-provider0
	    echo MODEMSUPPORTED=\"no\" >> etc/sysconfig/network/providers/dsl-provider0
	    echo ISDNSUPPORTED=\"no\" >> etc/sysconfig/network/providers/dsl-provider0
	    echo DSLSUPPORTED=\"yes\" >> etc/sysconfig/network/providers/dsl-provider0

	    echo USERNAME=`pppoed "user"` >> etc/sysconfig/network/providers/dsl-provider0
	    echo PASSWORD=`pppoed "password"` >> etc/sysconfig/network/providers/dsl-provider0
	    echo ASKPASSWORD=\"no\" >> etc/sysconfig/network/providers/dsl-provider0

	    echo DEMAND=`pppoed "demand"` >> etc/sysconfig/network/providers/dsl-provider0
	    echo IDLETIME=`pppoed "idle"` >> etc/sysconfig/network/providers/dsl-provider0
	    echo DNS1=`pppoed "dns1"` >> etc/sysconfig/network/providers/dsl-provider0
	    echo DNS2=`pppoed "dns2"` >> etc/sysconfig/network/providers/dsl-provider0

	fi
    fi

    if [ -r etc/wvdial.conf ] && `grep --quiet "\[Dialer modem0\]" etc/wvdial.conf` ; then
	if [ ! -r etc/sysconfig/network/ifcfg-modem0 ] ; then

	    echo "# generated during update" > etc/sysconfig/network/ifcfg-modem0

	    echo PROVIDER=\"ppp-provider0\" >> etc/sysconfig/network/ifcfg-modem0

	    echo STARTMODE=\"manual\" >> etc/sysconfig/network/ifcfg-modem0
	    echo BOOTPROTO=\"none\" >> etc/sysconfig/network/ifcfg-modem0

	    echo MODEM_DEVICE=`wvdial modem0 "Modem"` >> etc/sysconfig/network/ifcfg-modem0
	    echo SPEED=`wvdial modem0 "Baud"` >> etc/sysconfig/network/ifcfg-modem0

	    echo DIALCOMMAND=`wvdial modem0 "Dial Command"` >> etc/sysconfig/network/ifcfg-modem0
	    echo DIALPREFIX=`wvdial modem0 "Dial Prefix"` >> etc/sysconfig/network/ifcfg-modem0
	    echo INIT1=`wvdial modem0 "Init1"` >> etc/sysconfig/network/ifcfg-modem0
	    echo INIT2=`wvdial modem0 "Init2"` >> etc/sysconfig/network/ifcfg-modem0
	    echo INIT3=`wvdial modem0 "init3"` >> etc/sysconfig/network/ifcfg-modem0
	    echo INIT4=`wvdial modem0 "Init4"` >> etc/sysconfig/network/ifcfg-modem0
	    echo INIT5=`wvdial modem0 "Init5"` >> etc/sysconfig/network/ifcfg-modem0
	    echo INIT6=`wvdial modem0 "Init6"` >> etc/sysconfig/network/ifcfg-modem0
	    echo INIT7=`wvdial modem0 "Init7"` >> etc/sysconfig/network/ifcfg-modem0
	    echo INIT8=`wvdial modem0 "Init8"` >> etc/sysconfig/network/ifcfg-modem0
	    echo INIT9=`wvdial modem0 "Init9"` >> etc/sysconfig/network/ifcfg-modem0
	    echo PPPD_OPTIONS=`wvdial modem0 "PPPD Option"` >> etc/sysconfig/network/ifcfg-modem0

	    UNIQUEINFFILE="var/lib/YaST/unique.inf"
	    if [ -f $UNIQUEINFFILE ]; then
		while read inline; do
		    case $inline in
			\[Modem\]*)
			    while read netconf; do
				case $netconf in
				    \[*\]*)
					break
					;;
				    *)
					UNIQUESTR=`echo $netconf | gawk '{print $1}'`
					CONF=`echo $netconf | gawk '{print $2}'`
					CONFNUMBER=`echo $CONF | gawk -F';' '{print $2}'`
					if [ "$CONFNUMBER" = "" ]; then
					    CONFNUMBER="0";
					fi;
					CONFFILE="etc/sysconfig/network/ifcfg-modem0"
					if [ -f $CONFFILE ]; then
					    . $CONFFILE
					    if [ ! $UNIQUE ]; then
						echo "UNIQUE=\"$UNIQUESTR\"" >> $CONFFILE
					    fi;
					fi;
					;;
				esac
			    done
			;;
		    esac
		done < $UNIQUEINFFILE
	    fi

	fi
    fi

    for i in 0 1 2 3 4 5 6 7 8 9 ; do

	NAME=DIALER_NAME_$i
	ENTRY=DIALER_ENTRY_$i

	if [ -n "${!NAME}" ] && [ -n "${!ENTRY}" ] && [ -r etc/wvdial.conf ] ; then
	    if [ ! -r etc/sysconfig/network/providers/ppp-provider$i ] ; then

		echo "# generated during update" > etc/sysconfig/network/providers/ppp-provider$i
		chmod 0600 etc/sysconfig/network/providers/ppp-provider$i

		echo PROVIDER=\"${!NAME}\" >> etc/sysconfig/network/providers/ppp-provider$i
		echo MODEMSUPPORTED=\"yes\" >> etc/sysconfig/network/providers/ppp-provider$i
		echo ISDNSUPPORTED=\"no\" >> etc/sysconfig/network/providers/ppp-provider$i
		echo DSLSUPPORTED=\"no\" >> etc/sysconfig/network/providers/ppp-provider$i

		echo PHONE=`wvdial ${!ENTRY} "Phone"` >> etc/sysconfig/network/providers/ppp-provider$i
		echo USERNAME=`wvdial ${!ENTRY} "Username"` >> etc/sysconfig/network/providers/ppp-provider$i
		echo PASSWORD=`wvdial ${!ENTRY} "Password"` >> etc/sysconfig/network/providers/ppp-provider$i
		echo ASKPASSWORD=`wvdial_bool ${!ENTRY} "Ask Password"` >> etc/sysconfig/network/providers/ppp-provider$i

		echo STUPIDMODE=`wvdial_bool ${!ENTRY} "Stupid Mode"` >> etc/sysconfig/network/providers/ppp-provider$i
		echo COMPUSERVE=`wvdial_bool ${!ENTRY} "Compuserve"` >> etc/sysconfig/network/providers/ppp-provider$i

		echo DEMAND=`wvdial_bool ${!ENTRY} "Demand"` >> etc/sysconfig/network/providers/ppp-provider$i
		echo IDLETIME=`wvdial ${!ENTRY} "Idle Seconds"` >> etc/sysconfig/network/providers/ppp-provider$i
		echo DNS1=`wvdial ${!ENTRY} "DNS1"` >> etc/sysconfig/network/providers/ppp-provider$i
		echo DNS2=`wvdial ${!ENTRY} "DNS2"` >> etc/sysconfig/network/providers/ppp-provider$i

	    fi
	fi

    done

fi

