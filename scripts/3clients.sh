#!/bin/bash

./bin/benchmark_client --create=1 --start_create=0 --end_create=1000 &
./bin/benchmark_client --create=1 --start_create=1000 --end_create=2000 &
./bin/benchmark_client --create=1 --start_create=2000 --end_create=3000 &
./bin/benchmark_client --create=1 --start_create=3000 --end_create=4000 &

sleep 5

./bin/benchmark_client --set=1 --start_set=0 --num_set=3000 &
./bin/benchmark_client --set=1 --start_set=0 --num_set=3000 &
./bin/benchmark_client --set=1 --start_set=0 --num_set=3000 &

sleep 10