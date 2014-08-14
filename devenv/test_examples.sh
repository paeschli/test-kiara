#!/bin/bash

THIS_DIR=$(dirname "$0")

runtest() {
    local failed=
    kiara_calctest_server &
    pid=$!
    sleep 5

    kiara_calctest > result.txt || failed=yes
    kill -9 $pid > /dev/null 2> /dev/null
    [ "$failed" != yes ] && diff result.txt "$THIS_DIR/calctest_result.txt" || { failed=yes; }
    [ "$failed" = yes ] && { echo "Calc test failed"; return 1; } || { echo "Calc test: OK"; }

    kiara_structtest_server &
    pid=$!
    sleep 5
    kiara_structtest > result.txt || failed=yes
    kill -9 $pid > /dev/null 2> /dev/null
    [ "$failed" != yes ] && diff result.txt "$THIS_DIR/structtest_result.txt" || { failed=yes; }
    [ "$failed" = yes ] && { echo "Struct test failed"; return 1; } || { echo "Struct test: OK"; }

    kiara_remote_chat_server &
    pid=$!
    sleep 5
    kiara_remote_chat_client < "$THIS_DIR/chat_input.txt" > result.txt || failed=yes
    kill -9 $pid > /dev/null 2> /dev/null
    [ "$failed" != yes ] && diff result.txt "$THIS_DIR/chat_result.txt" || { failed=yes; }
    [ "$failed" = yes ] && { echo "Chat test failed"; return 1; } || { echo "Chat test: OK"; }

    kiara_enctest_server &
    pid=$!
    sleep 5
    kiara_enctest > result.txt || failed=yes
    kill -9 $pid > /dev/null 2> /dev/null
    [ "$failed" != yes ] && diff result.txt "$THIS_DIR/enctest_result.txt" || { failed=yes; }
    [ "$failed" = yes ] && { echo "Encryption test failed"; return 1; } || { echo "Encryption test: OK"; }

    rm result.txt
}

runtest
