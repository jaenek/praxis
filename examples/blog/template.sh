#!/bin/bash

TEMPLATE="$(cat template.html)"
OUTPUT="$(cat /dev/stdin)"
echo "${TEMPLATE/\[CONTENT\]/$OUTPUT}"
