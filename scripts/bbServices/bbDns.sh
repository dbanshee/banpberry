#! /bin/bash
#
# Banpberry Dns service script
#    Banshee -  2014
#

nArgs=$#

if [ $nArgs -lt 1 ] || [ $nArgs -gt 2 ] || [ $1 != "start" -a $1 != "stop" -a $1 != "restart" ] || ( [ $nArgs -eq 2 ] && [ $2 != "force" ] ) ; then
  echo "Usage : bbDns.sh <start|stop|restart> [force]"
  exit -1
fi

if [ $1 == "stop" ] || [ $1 == "restart" ] ; then
  if [ $nArgs -eq 2 ] ; then
    echo "Force Killing all bind9 processes ..."
    killall -s SIGKILL bind9
  else
   echo "Stoping Dns service  ..."
   service bind9 stop
  fi
fi

if [ $1 == "start" ] || [ $1 == "stop" ] ; then
  echo "Starting Dns service ..."
  service bind9 start
fi

