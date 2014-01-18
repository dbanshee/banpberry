#!/bin/bash
setterm -cursor off

# Cambiar opcion local por hdmi si se quiere usar la salida de audio digital
OPT="-o local"

omx_local() {
   #detectar subtitulos
   sub="${1%%.*}.str"
   if [ -f "$sub" ]
   then
      omxplayer ${OPT} --subtitles "$sub" "$1" | clear
   else
      omxplayer ${OPT} "$1" | clear
   fi
}

omx_remote() {
   START_TIME=$SECONDS
   omx_local "$1"
   ELAPSED_TIME=$(($SECONDS - $START_TIME))
   if [ "$ELAPSED_TIME" -lt 5 ]; then
      if [[ "$1" =~ 'http' ]]; then
         omxplayer ${OPT} $(youtube-dl --max-quality 35 -g "$1") | clear
      fi
   fi
}

for (( i=1;$i<=$#;i=$i+1 )) do 
   
   if [ -d "${!i}" ]
   then   
      for FILE in `find ${!i} -name "*.mp4" -o -name ".avi" -o -name "*.mkv" -type f`
      do
         omx_local "$FILE"
      done
   else 
      if [ -f "${!i}" ]
      then
         if [[ "${!i}" =~ ".lst" ]]
         then
            for line in $(cat ${!i}); do 
               omx_remote  "$line"
            done 
         else
            omx_local "${!i}"
         fi
      else 
         omx_remote  "${!i}"
      fi
   fi

done
setterm -cursor on