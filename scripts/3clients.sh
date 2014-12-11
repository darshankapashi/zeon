#!/bin/bash

./bin/benchmark_client --create=1 --start_create=0 --end_create=10000 &
./bin/benchmark_client --create=1 --start_create=10000 --end_create=20000 &
./bin/benchmark_client --create=1 --start_create=20000 --end_create=30000 &
#./bin/benchmark_client --create=1 --start_create=3000 --end_create=4000 &

#sleep 5

#./bin/benchmark_client --set=1 --start_set=0 --num_set=3000 &
#./bin/benchmark_client --set=1 --start_set=0 --num_set=3000 &
#./bin/benchmark_client --set=1 --start_set=0 --num_set=3000 &

#sleep 10
