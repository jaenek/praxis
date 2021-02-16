#!/bin/sh

echo '<main>'
grep -m 1 "# " -R "$1" \
	| sed 's!entries!!; s!\.md!\.html!; s!\#\ *!!' \
	| awk -F":" '{print "<header><a href=\""$1"\">"$2"</a></header>"}'
echo '</main>'
