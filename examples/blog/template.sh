#!/bin/bash

TEMPLATE="$(cat template.shtml)"
OUTPUT="$(cat /dev/stdin)"
echo "${TEMPLATE/\[CONTENT\]/$OUTPUT}"
