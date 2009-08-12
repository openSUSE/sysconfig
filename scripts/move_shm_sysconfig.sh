#!/bin/bash

root_dir="$1"
old_state="$root_dir/dev/shm/sysconfig"
new_state="$root_dir/dev/.sysconfig/network"
test -d "$new_state" || mkdir -p "$new_state" && \
for old_file in $(ls -1 "$old_state/" 2>/dev/null) ; do
	test -f "$old_state/$old_file" || continue
	new_file=$old_file
	case $old_file in
	new-stamp-*|ready-*)      ;;
	if-*|ifup-*|config-*)     ;;
	network) new_file=started ;;
	*)       continue         ;;
	esac
	if test -f "$new_state/$new_file" ; then
		rm -f "$old_state/$old_file"
	else
		mv -f "$old_state/$old_file" \
		      "$new_state/$new_file"
	fi
done
test -f "$new_state/started" && mkdir "$new_state/tmp"

