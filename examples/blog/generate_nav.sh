#!/bin/sh

for DIR in "$1"/*
do
	if [ -f "$DIR" ]
	then
		continue
	fi

	SUBDIR=${DIR##$(dirname "$DIR")/}
	printf '<a href=\"%s/index.shtml\">%s</a>\n' "/$SUBDIR" "$SUBDIR"
done
