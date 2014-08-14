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

startswith() {
    local s1=$1
    local s2=$2
    test "${s1:0:${#s2}}" = "$s2"
}

THIS_DIR=$(abspath_cd $(dirname "$0"))

if [ -z "$1" ]; then
    # We operate in root directory
    cd "$THIS_DIR/.."
    FILES=$(find . -name "*.hpp" -or -name "*.cpp" -or -name "*.py" -or -name "*.h" -or -name "*.c" | xargs grep -l Copyright | grep -v "tools/json" | grep -v "tools/mkcval.py" | grep -v "tools/maketest.py" | grep -v "tools/check.py")
else
    FILES="$@"
fi

for f in $FILES; do
    perl -w -e '
my $remove_copyright=0;
while (<>) {
    my $fn = $_;
    if ($remove_copyright eq 0) {
        if (/\/\*  KIARA - Middleware for efficient and QoS\/Security-aware invocation of services and exchange of messages/) {
            $remove_copyright = 1;
            next;
        }
    } elsif ($remove_copyright eq 1) {
        if (/ \*\//) {
            $remove_copyright = 2;
            next;
        }
        next;
    }
    print;
}
'< "$f" > "${f}.tmp" || exit 1
    cat "${f}.tmp" > "$f" || exit 1
    rm "${f}.tmp" || exit 1
done
