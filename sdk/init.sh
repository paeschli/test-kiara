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

THIS_DIR=$(abspath_cd "$(dirname "$0")")
PATH=$PATH:$THIS_DIR/bin
if [[ "$OS" == "Windows"* ]]; then
    PATH=$PATH:$THIS_DIR/lib
else
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$THIS_DIR/lib
fi
KIARA_MODULE_PATH=$THIS_DIR/lib
export PATH LD_LIBRARY_PATH KIARA_MODULE_PATH

echo "KIARA SDK Environment"
kiara-version || {
    echo "Could not run kiara-version tool, something is wrong."
    exit 1
}

exec bash -i
