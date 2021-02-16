#!/bin/sh

echo '<nav>'
for DIR in "$1"/*
do
	if [ -f "$DIR" ]
	then
		continue
	fi

	SUBDIR=${DIR##$(dirname "$DIR")/}
	printf '<a href=\"%s/index.html\">%s</a>\n' "/$SUBDIR" "$SUBDIR"
done
echo '</nav>'
