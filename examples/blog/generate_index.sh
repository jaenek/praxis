#!/bin/sh

echo '<main>'
find "$1"/* -type f -name "*[^(index)].html" -exec grep -Hm 1 '<h1>.*</h1>' {} + \
	| sed 's!public!!; s!</*h1>!!g' \
	| awk -F":" '{ print "<header><a href=\""$1"\">"$2"</a></header>"}'
echo '</main>'
