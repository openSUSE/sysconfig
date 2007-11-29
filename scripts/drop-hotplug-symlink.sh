#!/bin/bash

# Drop the compatibility symbolic link in /etc/sysconfig/network,
# usually as ifcfg-eth-pcmcia -> ifcfg-eth-pcmcia-0,

path=${root}/etc/sysconfig/network
types="eth-pcmcia eth-usb tr-pcmcia tr-usb"

# path=/tmp/n
cd "$path"

for t in $types; do
  f="ifcfg-$t"
  test -L "$f" || continue
  s="$(readlink "$f")"
  test -f "$s" || continue
  echo "$f -> $s"
  rm -f "$f"
  mv -f "$s" "$f"
done
