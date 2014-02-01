#! /bin/bash
#
# Banpberry Omxplayer service script
#    Banshee -  2014
#

nArgs=$#

if [ $nArgs -lt 1 ] || [ $nArgs -gt 2 ] || [ $1 != "start" -a $1 != "stop" -a $1 != "restart" ] || ( [ $nArgs -eq 2 ] && [ $2 != "force" ] ) ; then
  echo "Usage : bbVlc.sh <start|stop|restart> [force]"
  exit -1
fi

if [ $1 == "stop" ] || [ $1 == "restart" ] ; then
  if [ $nArgs -eq 2 ] ; then
    echo "Force Killing all omxplayer.bin processes ..."
    killall -s SIGKILL omxplayer.bin
  else
   echo "Stoping omxplayer.bin processes  ..."
   killall -s SIGKILL omxplayer.bin
  fi
fi

if [ $1 == "start" ] || [ $1 == "restart" ] ; then
  echo "Start Omxplayer service Not Implemented. Take a look to bbMediaServer of Banpberry Servers."
fi

