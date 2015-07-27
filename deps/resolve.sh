#!/bin/bash

BASE=$PWD
DEPS=`ls`

for ITEM in $DEPS; do
	TARGET="$BASE/$ITEM"
	echo "$TARGET"
	if [ -d "$TARGET" ]; then
		cd "$TARGET"
		git pull origin master
	fi
done
