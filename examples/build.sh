#!/bin/bash

gcc -o structtest structtest.c -I../include -L../lib -lKIARA

gcc -o structtest_server structtest_server.c -I../include -L../lib -lKIARA

export KIARA_MODULE_PATH=$PWD/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/lib
