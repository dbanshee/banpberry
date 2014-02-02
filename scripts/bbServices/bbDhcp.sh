#! /bin/bash
#
# Banpberry Dhcp service script
#    Banshee -  2014
#

nArgs=$#

if [ $nArgs -lt 1 ] || [ $nArgs -gt 2 ] || [ $1 != "start" -a $1 != "stop" -a $1 != "restart" ] || ( [ $nArgs -eq 2 ] && [ $2 != "force" ] ) ; then
  echo "Usage : bbDhcp.sh <start|stop|restart> [force]"
  exit -1
fi

if [ $1 == "stop" ] || [ $1 == "restart" ] ; then
  if [ $nArgs -eq 2 ] ; then
    echo "Force Killing all dhcpd processes ..."
    killall -s SIGKILL dhcpd
  else
   echo "Stoping Dhcp service  ..."
   service isc-dhcp-server stop
  fi
fi

if [ $1 == "start" ] || [ $1 == "stop" ] ; then
  echo "Starting Dhcp service ..."
  service isc-dhcp-server start
fi

