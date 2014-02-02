#! /bin/bash
#
# Banpberry StartX service script
#    Banshee -  2014
#

nArgs=$#

if [ $nArgs -lt 1 ] || [ $nArgs -gt 2 ] || [ $1 != "start" -a $1 != "stop" -a $1 != "restart" ] || ( [ $nArgs -eq 2 ] && [ $2 != "force" ] ) ; then
  echo "Usage : bbStartX.sh <start|stop|restart> [force]"
  exit -1
fi

if [ $1 == "stop" ] || [ $1 == "restart" ] ; then
  if [ $nArgs -eq 2 ] ; then
    echo "Force Killing all startx services ..."
    killall -s SIGKILL startx
  else
   echo "Stoping startx service may be only by bbControlServer command. If not runs try FORCE option."
   killall startx
  fi
fi

if [ $1 == "start" ] || [ $1 == "restart" ] ; then
  echo "Starting startx service ..."
  startx &> /dev/null & # Investigar si no lanzarlo en 2 plano y controlarlo mediante stdout y enviar el CTRL^C para cierre controlado
fi

