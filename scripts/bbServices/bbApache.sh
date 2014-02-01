#! /bin/bash
#
# Banpberry Apache service script
#    Banshee -  2014
#

nArgs=$#

if [ $nArgs -lt 1 ] || [ $nArgs -gt 2 ] || [ $1 != "start" -a $1 != "stop" -a $1 != "restart" ] || ( [ $nArgs -eq 2 ] && [ $2 != "force" ] ) ; then
  echo "Usage : bbApache.sh <start|stop|restart> [force]"
  exit -1
fi

if [ $1 == "stop" ] || [ $1 == "restart" ] ; then
  if [ $nArgs -eq 2 ] ; then
    echo "Force Killing all apache services ..."
    killall -s SIGKILL apache2
  else
   echo "Stoping apache service  ..."
   service apache2 stop
  fi
fi

if [ $1 == "start" ] || [ $1 == "stop" ] ; then
  echo "Starting apache2 service ..."
  service apache2 start
fi

