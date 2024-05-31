#!/bin/sh
##@file parsePy.sh
##
## Parsing every Py file
##
for var in "$@"
do
    grep -E "#" $var >> "all_comments.tmp"
done