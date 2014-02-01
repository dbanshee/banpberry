#! /bin/bash
#
# Banpberry Vnc service script
#    Banshee -  2014
#

nArgs=$#

if [ $nArgs -lt 1 ] || [ $nArgs -gt 2 ] || [ $1 != "start" -a $1 != "stop" -a $1 != "restart" ] || ( [ $nArgs -eq 2 ] && [ $2 != "force" ] ) ; then
  echo "Usage : bbVnc.sh <start|stop|restart> [force]"
  exit -1
fi

if [ $1 == "stop" ] || [ $1 == "restart" ] ; then
  if [ $nArgs -eq 2 ] ; then
    echo "Force Killing all vnc processes ..."
    killall -s SIGKILL x11vnc
  else
   echo "Stoping vncs processes  ..."
   killall -s SIGKILL x11vnc
  fi
fi

if [ $1 == "start" ] || [ $1 == "restart" ] ; then
  x11vnc -bg -usepw -noipv6 & # Investigar si no lanzarlo en 2 plano y controlarlo mediante stdout y enviar el CTRL^C para cierre controlado
fi

