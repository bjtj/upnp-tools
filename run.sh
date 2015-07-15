#!/bin/bash

PATH=$PWD/world/bin:$PATH
PROGRAM=upnp-tools

OPT=run

if [ -n "$1" ]; then
	OPT=$1
	shift
fi

case $OPT in
	run)
		$PROGRAM $@
		;;
	gdb)
		gdb --args $PROGRAM $@
		;;
	check)
		valgrind --leak-check=yes $PROGRAM $@
		;;
	*)
		echo "unknown option -- $OPT"
		exit 1
		;;
esac
