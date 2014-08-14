#!/bin/bash

THIS_DIR=$(dirname "$0")

GPL_LIC="$THIS_DIR/../doc/templates/header.gpl"
LGPL_LIC="$THIS_DIR/../doc/templates/header.lgpl"
LICFILE=

declare -i end_of_options=0
years=$(date +%Y)

fixFile()
{

    if [ -z "$LICFILE" ]; then
        echo >&2 "No license specified"
        exit 1
    fi

    for f; do
        if grep Copyright "$f" >/dev/null; then
            echo >&2 "Copyright notice in file $f, don't do anything !!!!"
        fi
        sed "s|<year>|$years|g;s|<name of author>|$author|g;" "$LICFILE"  > "${f}.tmp" && \
            cat "$f" >>  "${f}.tmp" && \
            mv "${f}.tmp" "$f"
        echo "Modified $f"
    done
}

while [ $# -ne 0 ]; do
    case "$1" in
	--)
	    end_of_options=1
	    ;;
	--gpl|--GPL)
            LICFILE=$GPL_LIC
            ;;
	--lgpl|--LGPL)
            LICFILE=$LGPL_LIC
            ;;
	--year=*)
            years="${1:7}"
            if [ -z "$years" ]; then
                echo >&2 "No year(s) specified in option: $1 (e.g. --year=2010)"
                exit 1
            fi
	    ;;
        --author=*)
            author=${1:9}
	    if [ -z "$author" ]; then
                echo >&2 "No author specified in option: $1 (e.g. --author=Name)"
	        exit 1
	    fi
            ;;
	-*)
	    echo >&2 "unknown option $1"
	    exit 1
	    ;;
	*)
            fixFile "$1"
            ;;
    esac
    shift
    [ $end_of_options -eq 1 ] && break
done
