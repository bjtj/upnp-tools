#!/bin/bash

BASE=$PWD
DIR_BUILD=$PWD/build
DIR_WORLD=$PWD/world

OPT=build

CPU_COUNT=$(cat /proc/cpuinfo | grep processor | wc -l)

if [ -n "$1" ]; then
	OPT=$1
fi

clean() {
	rm -rf $DIR_BUILD $DIR_WORLD
}

reconf() {
	autoreconf -i
}

reconf_if_need() {
	if [ ! -f configure ]; then
		reconf
	fi
}

build() {
	mkdir -p $DIR_BUILD
	mkdir -p $DIR_WORLD
	cd $DIR_BUILD
	$BASE/configure --prefix="$DIR_WORLD" --enable-debug
	make -j$CPU_COUNT
	make install
}

build_tui() {
	mkdir -p $DIR_BUILD
	mkdir -p $DIR_WORLD
	cd $DIR_BUILD
	$BASE/configure --prefix="$DIR_WORLD" --enable-debug --with-ncurses
	make -j$CPU_COUNT
	make install
}

install() {
	clean
	mkdir -p $DIR_BUILD
	cd $DIR_BUILD
	$BASE/configure
	make -j$CPU_COUNT
	sudo make install
}

check() {
	cd $DIR_BUILD
	make check -j$CPU_COUNT
}

echo "cpu count := $CPU_COUNT"

case $OPT in
	reconf)
		reconf
		;;
	build)
		reconf_if_need
		build
		;;
	build-tui)
		reconf_if_need
		build_tui
		;;
	all)
		prepare
		build
		;;
	tags)
		cd $DIR_BUILD && make tags
		;;
	make)
		cd $DIR_BUILD && make && make install
		;;
	install)
		reconf_if_need
		install
		;;
	check)
		check
		;;
	dist)
		cd $DIR_BUILD && make dist
		;;
	clean)
		clean
		;;
	*)
		echo "unknown option - $OPT"
		exit 1
		;;
esac
