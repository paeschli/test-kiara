#!/bin/bash

abspath()
{
    if [ -d "$1" ]; then
        echo "$(cd "$1"; pwd)";
    else
        case "$1" in
            "" | ".")
                echo "$PWD"
            ;;
            /*)
                echo "$1"
            ;;
            *)
                echo "$PWD/$1"
            ;;
        esac;
    fi
}

THIS_DIR=$(abspath "$(dirname "$0")")

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

runValgrind() {
    valgrind $OPT_ARG --num-callers=50 --track-origins=yes  --show-reachable=yes --leak-resolution=high --leak-check=full --error-exitcode=2 --gen-suppressions=all "$@"
}

setArrayElem ()
{
    eval "$1[$2]=\"$3\""
}

getArrayLength()
{
    eval "echo \${#$1[@]}"
}

getArrayElem()
{
    eval "echo \"\${$1[$2]}\""
}

appendArrayElem()
{
    setArrayElem "$1" "$(getArrayLength "$1")" "$2"
}

echoError() {
    echo >&2 "$@"
}

ERRORS=()

processFiles() {
    local f fn
    for f in "$@"; do
        case "$f" in
           #*".kl.out")
           #     fn=${f:0:${#f}-4}
            *".kl")
                fn=$f
                if [ -f "$fn" ]; then
                    echo -ne "Process $fn"
                    if ! runValgrind kiara-lang "$fn" >"$fn.output" 2>"$fn.valgrind"; then
                        echo " FAILED"
                        echoError "Valgrind detected errors while executing kiara-lang $fn"
                        appendArrayElem ERRORS "$fn"
                    else
                        echo " OK"
                        rm "$fn.valgrind" "$fn.output"
                    fi
                fi
                ;;
            *)
                if [[ -f "$f" && -x "$f" && ! ("$f" == *.so || "$f" == *.dll) ]]; then
                    echo -ne "Process $f"
                    if ! runValgrind "$f" > "$f.output" 2>"$f.valgrind"; then
                        echo " FAILED"
                        echoError "Valgrind detected errors while executing $f"
                        appendArrayElem ERRORS "$f"
                    else
                        echo " OK"
                        rm "$f.valgrind" "$f.output"
                    fi
                fi
                ;;
        esac
    done
}

for f in "$@"; do
    if [ -f "$f" ]; then
        processFiles "$f";
    elif [ -d "$f" ]; then
        for ff in $(find "$f" -maxdepth 1 | sort); do
            processFiles "$ff"
        done
    fi
done

if [ "$(getArrayLength ERRORS)" -ne 0 ]; then
    echo "Failed valgrind tests:"
    for ((i=0;i<$(getArrayLength ERRORS);i++)); do
        echo "$(getArrayElem ERRORS "$i")"
    done
    exit 1
fi

exit 0
