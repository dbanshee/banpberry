#! /bin/bash

# 
# X11 intercepta las señales (SIGTERM, SIGKILL, ...) que no provienen desde la entrada estandard,
# Por lo que si es lanzado en 2 plano no es posible finalizarlo a excepcion de la interfaz grafica o
# mediante ctrl+alt+backspace.
#
# Investigar lanzamiento 
#   echo "startx" | bash &
#  y posteriormente realizar kill sobre el bash. 
#


echo "Inicializando X11 Server ..."
startx &
sleep 5

# x11vnc muere automaticamente al terminar startx
echo "Inicializando VNC (port 5900) ... "
startVNC.sh
