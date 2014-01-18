#! /bin/bash

if [ $# -lt 2 ] ; then
  echo "Usage : ext4mount.sh <ntfs4dev> <mountpoint>"
fi

sudo mount -t ntfs $1 $2
