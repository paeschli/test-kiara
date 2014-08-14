#!/bin/bash

OPTS="-O0 -g -Wall"

if [ ! -x ./lemon ]; then
    g++ -Wall -O2 lemon.c -olemon || exit 1
fi
./lemon H=kiaray_tokens.hpp O=kiaray.cpp kiaray.yy || exit 1
flex -o kiaral.cpp kiaral.ll || exit 1
#flex -o kiaral.cpp --header-file=kiaral.hpp kiaral.ll
# cpp $OPTS kiaral.cpp > kiaral.out.cpp
# g++ $OPTS -c IDLParserContext.cpp || exit 1
# g++ $OPTS -c kiaral.cpp || exit 1
# g++ $OPTS -c parsertest.cpp || exit 1
# g++ $OPTS -c kiaraparser.cpp || exit 1
# g++ $OPTS kiaral.o parsertest.o kiaraparser.o -o parsertest || exit 1
scons
