#! /bin/bash
#
# Banpberry Mopidy service script
#    Banshee -  2014
#

nArgs=$#

if [ $nArgs -lt 1 ] || [ $nArgs -gt 2 ] || [ $1 != "start" -a $1 != "stop" -a $1 != "restart" ] || ( [ $nArgs -eq 2 ] && [ $2 != "force" ] ) ; then
  echo "Usage : bbMopidy.sh <start|stop|restart> [force]"
  exit -1
fi

if [ $1 == "stop" ] || [ $1 == "restart" ] ; then
  if [ $nArgs -eq 2 ] ; then
    echo "Force Killing all mopidy processes ..."
    killall -s SIGKILL mopidy
  else
   echo "Killing all mopidy processes ..."
   killall mopidy
  fi
fi

if [ $1 == "start" ] || [ $1 == "restart" ] ; then
  echo "Starting mopidy service ..."
  /usr/local/bin/mopidy --config /home/pi/.config/mopidy/mopidy.conf &> /var/log/mopidy.log &
fi

