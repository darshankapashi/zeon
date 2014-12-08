#!/bin/bash

# The config file has the following format
# Followed by N lines
# <nid>,<ip>,<serverPort>,<clientPort>
# ....

# The saved config is the following format
# <Node region map if it exists> - N lines
# x1,y1,x2,y2,main nid, replica nid, ..
# ...

CONFIG_FILE="config.txt"
SAVED_CONFIG_FILE="saved_config.txt"

if [ ! -f $CONFIG_FILE ]; then
  echo "Config file not found: $CONFIG_FILE"
  exit 1
fi

trap : SIGTERM SIGINT
echo $$

pkill -f bin/leader
pkill -f bin/server
rm -rf /tmp/zeon-*
mkdir /tmp/zeon-points
mkdir /tmp/zeon-values

echo "Starting servers..."

while read line
do
  while IFS=',' read -ra ADDR; do
    ./bin/server --my_nid=${ADDR[0]} --server_talk_port=${ADDR[2]} --client_port=${ADDR[3]} &
  done <<< "$line"
done < $CONFIG_FILE

echo "Started servers. Waiting 2 seconds before starting leader."

sleep 2
./bin/leader &
leader=$!

echo "Started leader."

wait $leader

if [[ $? -gt 128 ]]
then
  echo "Shutting down..."
  pkill -f bin/server
  kill $leader
fi