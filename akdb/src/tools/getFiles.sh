#!/bin/bash
##@file getFiles.sh
##
##Finding all files that ends with extension .py or .c and storing them into file.txt
##
find -regex ".*\.\(c\|py\)" > files.txt