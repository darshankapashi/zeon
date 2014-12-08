#!/bin/bash 

trap : SIGTERM SIGINT
echo $$

rm -rf /tmp/zeon-*/*
./bin/leader &
leader=$!
./bin/server --my_nid=1 --server_talk_port=9000 --client_port=8000 &
server1=$!
./bin/server --my_nid=2 --server_talk_port=9001 --client_port=8001 &
server2=$!

wait $leader

if [[ $? -gt 128 ]]
then
  echo "Shutting down..."
  kill $server1
  kill $server2
  kill $leader
fi
