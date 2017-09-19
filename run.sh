#!/usr/bin/env sh
./bin/main c 1

echo "creating done"
sleep 2

./bin/main r 2 &
./bin/main a 3 &
./bin/main a 4 &
./bin/main a 5 &
./bin/main a 6 &
./bin/main a 7 &
./bin/main a 8

sleep 2

./bin/main v 9
