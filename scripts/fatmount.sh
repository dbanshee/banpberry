#! /bin/bash

if [ $# -lt 2 ] ; then
  echo "Usage : fatmount.sh <fatdev> <mountpoint>"
fi

sudo mount -t vfat $1 $2
