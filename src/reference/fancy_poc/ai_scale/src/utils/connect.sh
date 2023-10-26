
#!/bin/bash
set -x

res_folder="res"
flag_folder="flag"

if [ -d "$res_folder" ]; then
  rm -rf "$res_folder"/*
else
  mkdir "$res_folder"
fi

if [ -d "$flag_folder" ]; then
  rm -rf "$flag_folder"/*
else
  mkdir "$flag_folder"
fi

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <IP_ADDRESS> <PORT>"
  exit 1
fi

ip_address="$1"
port="$2"

./client "$ip_address" "$port"
