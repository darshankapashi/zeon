#!/bin/bash

for i in `seq 1 $1`
do
  ./bin/benchmark_client --create=1 --start_create=$(((i - 1) * 10000)) --end_create=$((i * 10000)) &
done

#sleep 5

#./bin/benchmark_client --set=1 --start_set=0 --num_set=3000 &
#./bin/benchmark_client --set=1 --start_set=0 --num_set=3000 &
#./bin/benchmark_client --set=1 --start_set=0 --num_set=3000 &

#sleep 10
