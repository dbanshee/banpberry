#! /bin/bash

if [ $# -lt 1  ] ; then
  echo "Usage : remoteCmd.sh <command> [arg]*"
fi

DISPLAY=:0.0 $@
