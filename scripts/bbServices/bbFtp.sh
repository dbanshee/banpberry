#! /bin/bash
#
# Banpberry Vsftpd service script
#    Banshee -  2014
#

nArgs=$#

if [ $nArgs -lt 1 ] || [ $nArgs -gt 2 ] || [ $1 != "start" -a $1 != "stop" -a $1 != "restart" ] || ( [ $nArgs -eq 2 ] && [ $2 != "force" ] ) ; then
  echo "Usage : bbVsftpd.sh <start|stop|restart> [force]"
  exit -1
fi

if [ $1 == "stop" ] || [ $1 == "restart" ] ; then
  if [ $nArgs -eq 2 ] ; then
    echo "Force Killing all vsftpd services ..."
    killall -s SIGKILL vsftpd
  else
   echo "Stoping vsftpd service  ..."
   service vsftpd stop
  fi
fi

if [ $1 == "start" ] || [ $1 == "restart" ] ; then
  echo "Starting vsftpd service ..."
  service vsftpd start
fi

