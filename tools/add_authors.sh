#!/bin/bash

abspath_cd() {
    if [ -d "$1" ]; then
        echo "$(cd "$1"; pwd)"
    else
        case "$1" in
            "" | ".") echo "$PWD";;
            /*) echo "$1";;
            *)  echo "$PWD/$1";;
        esac
    fi
}

THIS_DIR=$(abspath_cd $(dirname "$0"))

declare -i END_OF_OPTIONS=0

while [ $# -ne 0 ]; do
    case "$1" in
	--)
	    END_OF_OPTIONS=1
	    ;;
	-*)
	    echo >&2 "unknown option $1"
	    exit 1
	    ;;
    esac
    shift
    [ $END_OF_OPTIONS -eq 1 ] && break
done

# ask prompt default
ask() {
    local prompt=$1
    local default=$2

    echo "$1"
    [ -n "$default" ] && echo -n "[$default] "
    read -e

    [ -z "$REPLY" ] && REPLY=$default
}

THIS_DIR=$(abspath_cd $(dirname "$0"))
if [ -z "$1" ]; then
    # We operate in root directory
    cd "$THIS_DIR/.."
    FILES=$(find . -name "*.hpp" -or -name "*.cpp" -or -name "*.py" \
        | xargs grep -L "Author:\|Copyright"  | grep -v "tools/json" \
        | grep -v "tools/mkcval.py" | grep -v "tools/maketest.py" \
        | grep -v "tools/check.py")
else
    FILES="$@"
fi

for f in $FILES; do
    # First author
    INFO_STR=$(git log --pretty="format:%an|%ae|%ai|%s" "$f" | tail -n1)

    if [ -z "$INFO_STR" ]; then
        continue
    fi

    echo "******* Processing $f *******"

    oldIFS=$IFS
    IFS="|"
    INFOS=($INFO_STR)
    IFS=$oldIFS

    AUTHOR=${INFOS[0]}
    AUTHOR_EMAIL=${INFOS[1]}
    CREATION_TIME=${INFOS[2]}

    oldIFS=$IFS
    IFS="
"

    TMP=($(git log --reverse --pretty="format:%an" "$f" | uniq))
    IFS=$oldIFS

    ALL_AUTHORS=()
    for ((i=0; i<${#TMP[*]}; i++)); do
        # Fixes
        if [ "${TMP[$i]}" = "Kristian" ]; then
            TMP[$i]="Kristian Sons"
        fi

        # Check in order to avoid adding the same author
        # multiple times
        ADD_AUTHOR=1
        for ((j=0; j<${#ALL_AUTHORS[*]}; j++)); do
            if [ "${TMP[$i]}" = "${ALL_AUTHORS[j]}" ]; then
                ADD_AUTHOR=0
                break
            fi
        done

        [ $ADD_AUTHOR -eq 0 ] && continue

        ALL_AUTHORS[${#ALL_AUTHORS[*]}]=${TMP[$i]}
    done

    unset TMP

    ALL_AUTHORS_STR=""
    for ((i=0; i<${#ALL_AUTHORS[*]}; i++)); do
        if [ $i -ne $(( ${#ALL_AUTHORS[*]}-1 )) ]; then
            ALL_AUTHORS_STR="${ALL_AUTHORS_STR}${ALL_AUTHORS[$i]}, "
        else
            ALL_AUTHORS_STR="${ALL_AUTHORS_STR}${ALL_AUTHORS[$i]}"
        fi
    done

    BASENAME_F=$(basename "$f")

    echo "/*
 * $BASENAME_F
 *
 *  Created on: $CREATION_TIME
 *   Author(s): $ALL_AUTHORS_STR
 */
" > "${f}.tmp" || exit 1
    cat "$f" >> "${f}.tmp" || exit 1
    cat "${f}.tmp" > "$f" || exit 1
    rm "${f}.tmp" || exit 1
done
