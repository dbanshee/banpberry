#! /bin/bash

setterm -cursor off    # Elimina laterales de consola. 
omxplayer -o hdmi "$@"
xrefresh -display :0     # Solve blackscreen after playback
setterm -cursor on
