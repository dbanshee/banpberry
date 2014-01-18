#! /bin/bash

if [ $# -lt 1  ] ; then
  echo "Usage : isomount.sh <isofile>"
fi


sudo mount -o loop $1 /media/iso
