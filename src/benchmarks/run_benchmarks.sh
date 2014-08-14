#!/bin/bash

THIS_DIR=$(dirname "$0")
cd "$THIS_DIR"

runBenchmark() {
  local i
  echo "Testing: $@"
  echo > test_result.txt
  for ((i = 0; i < 10; i++)) {
    echo -n "."
    eval "$@" >> test_result.txt
  }
  awk '/Average latency/ {ms = $4; n+=1; sum+=$5} END { print "Computed",n,"samples"; print "Average latency in", ms,sum/n; if (ms ~ /milliseconds/) { print "Average latency in microseconds:", (sum/n)*1000.0 ; } }' test_result.txt
  echo
}

if [ -n "$1" ]; then
    runBenchmark "$@"
    exit $?
fi

if true; then
runBenchmark "perl tools/boost_test.pl Boost 1000 1000"
runBenchmark "perl tools/boost_test.pl BoostTyped 1000"
runBenchmark "perl tools/boost_test.pl BoostTyped2 1000"
fi

echo "Starting KIARA server"
KiaraTypedSubscriber 8080 ortecdr > server_result.txt &
ppid=$!

while ! grep "Starting" server_result.txt >/dev/null  2>&1; do
  sleep 1
done
sleep 1

echo "Running KIARA clients"

runBenchmark "KiaraTypedPublisher"
kill -s SIGTERM $ppid
