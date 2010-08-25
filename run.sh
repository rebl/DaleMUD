#!/bin/sh
#

export MALLOC_CHECK_=0
# ./dmserver "$@"
gdb -d src ./dmserver
