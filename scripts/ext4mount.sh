#! /bin/bash

if [ $# -lt 2 ] ; then
  echo "Usage : ext4mount.sh <ext4dev> <mountpoint>"
fi

sudo mount -t ext4 $1 $2
