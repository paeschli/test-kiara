#!/bin/bash

THIS_DIR=$(dirname "$0")

if [ -e "$THIS_DIR/valgrind/llvm.sup" ]; then
    OPT_ARG="--suppressions=$THIS_DIR/valgrind/llvm.sup"
else
    OPT_ARG=
fi

if [ -e "$THIS_DIR/valgrind/curl.sup" ]; then
    OPT_ARG="$OPT_ARG --suppressions=$THIS_DIR/valgrind/curl.sup"
fi

if [ -e "$THIS_DIR/valgrind/boost.sup" ]; then
    OPT_ARG="$OPT_ARG --suppressions=$THIS_DIR/valgrind/boost.sup"
fi

valgrind $OPT_ARG --num-callers=50 --track-origins=yes  --show-reachable=yes \
    --leak-resolution=high --leak-check=full --error-exitcode=2 --gen-suppressions=all "$@"  2>&1 | \
    tee leaks.txt && "$THIS_DIR/make_ptr_uids.py" leaks.txt
