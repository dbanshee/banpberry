#!/usr/bin/env python 

#----------------------------------------------------
# Banpberry omxplayer Control Server
#  Prueba de concepto
#
#  Servidor por socket para control de omxplayer.
#----------------------------------------------------


# Investigar sobre libreria twisted sobre procesos. http://twistedmatrix.com/documents/9.0.0/core/howto/process.html
# Investigar Threading. Ejecutar omxplayer en thread aparte para intentar evitar el bloqueo de socket

import datetime
import socket
import os
import subprocess
import signal
import sys
import threading
import traceback
import time

"""
" Formato de excepciones
"""
def printException():
  print '\n\n>>> traceback <<<'
  traceback.print_exc(file=sys.stdout)
  print '>>> end of traceback <<<\n\n'
  sys.stdout.flush()
  sys.stderr.flush()

"""
" Manejadores de senales del SO
"""
def handleSigTERM(signal, frame):
  print '\n Closing bbOmxControlServer ...'  
  closeConnection()
  sys.exit(0)
    
  
def handleSigINT(signal, frame):
  handleSigTERM()  

"""
" Cierre controlado de conexion
"""
def closeConnection():
  global c
  if c != None:
    try:
      print "<" + str(datetime.datetime.now()) + "> Trying to close connection."
      c.send('\nConnection End.\n')
      
      # Aunque se haga el close de la conexion no se cierra realmente por alguna relacion entre conexion y pipes.
      c.close()                
      print "<" + str(datetime.datetime.now()) + "> Connection closed sucessfull."
    except:
      print "<" + str(datetime.datetime.now()) + "> Error closing connection."
  else:
    print "<" + str(datetime.datetime.now()) + "> No connection Pendings."
    
  c = None
   

"""
" Ejecuta el proceso en una pipe
"""
def execPiped(url):
  global process
  process = subprocess.Popen(['omxplayer', '-o', 'hdmi', url], shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)

"""
" Manejadores de peticiones
"""

def playHandler(sock, uri):
  global process
  global trackVideo
 
  # No se permite por el momento mas de una instancia de omxplayer
  killOmxplayer()
  
  print "   Playing Media : '" + uri + "'"
  
  try:
    if 'youtube.com' in uri:
      
      print '    Youtube link detected. Recovering stream url ... '
      
      #linkSent = 'youtube-dl --max-quality 35 -g ' + uri
      #p = subprocess.Popen(linkSent,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
      p = subprocess.Popen(['youtube-dl', '--max-quality', '35', '-g', uri], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      output, errors = p.communicate()

      # Es posible que sea necesario un tiempo de espera para que el link este disponible.
      #time.sleep(5);
      
      if errors:
        print '      Stderr : ' + errors
      
      if output:
        finalUri = output.strip('\n\t\r')

        # omxplayer no reproduce correctamente https, sin embargo la media esta disponible a traves de http tambien.
        finalUri = finalUri.replace('https://', 'http://')
      else:
        print '       Error recovering stream url for uri : ' + uri
    else:
      finalUri = uri
    
    osSent = "omxplayer -o hdmi '" + finalUri + "'"
    print '     OSExec : ' + osSent
    
    trackVideo = uri
    
    #os.system(osSent)
    #process = subprocess.Popen(['omxplayer', '-o', 'hdmi', url], shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    # Se ejecuta en un Thread independiente para desacoplarlo de la relacion entre socket TCP y los ficheros de Pipes abiertos. De lo contrario no se libera la conexion TCP en el cliente hasta que omxplayer termine. (Investigar motivo)
    t = threading.Thread(target=execPiped, args=(finalUri,))
    t.start()
    
    sock.send("  Playing Media : '" + uri + "'")
    print '    Subproceso omxplayer lanzado.'
  
  except:  
    print 'Error launching omxplayer uri : '+ uri
    printException()
    
    killOmxplayer()
    
def pauseHandler(sock):
  global process
  
  if process != None:
      print '   PAUSE'
      
      
      print '   Sending <space> to omxplayer ...'
      process.stdin.write(' ')
      process.stdin.flush()

      print 'Player PAUSED'
      sock.send("  PAUSE")
  else:
    print '    PAUSE ignored. Omxplayer NOT Running'


def closeHandler(sock):
  global process
  
  if process != None:
      print '   CLOSE'
      killOmxplayer()
      sock.send("  CLOSE")
  else:
    print '    CLOSE ignored. Omxplayer NOT Running'

  
def volumeHandler(sock, mode):
  global process
  
  if process != None:
    print '   VOL-' + mode  
    
    if mode == "UP":
      print '   Sending + to omxplayer ...'
      process.stdin.write('+')
    elif mode == "DOWN":
      print '   Sending - to omxplayer ...'
      process.stdin.write('-')
    
    process.stdin.flush()
    
    sock.send("  VOL-" + mode)
  else:
    print '    VOL-' + mode + ' ignored. Omxplayer NOT Running'
    
def speedHandler(sock, mode):
  global process
  
  if process != None:
    print '   SPEED-' + mode  
    
    if mode == "UP":
      print '   Sending 1 to omxplayer ...'
      process.stdin.write('1')
    elif mode == "DOWN":
      print '   Sending 2 to omxplayer ...'
      process.stdin.write('2')
    
    process.stdin.flush()
    
    sock.send("  SPEED-" + mode)
  else:
    print '    SPEED-' + mode + ' ignored. Omxplayer NOT Running'
    
def motionHandler(sock, mode):
  global process
  
  if process != None:
    print '   MOTION-' + mode  
    
    if mode == "F": # Fast Forward
      print '   Sending > to omxplayer ...'
      process.stdin.write('>')
    elif mode == "R": # Rewind
      print '   Sending < to omxplayer ...'
      process.stdin.write('<')
    
    process.stdin.flush()
    
    sock.send("  MOTION-" + mode)
  else:
    print '    MOTION-' + mode + ' ignored. Omxplayer NOT Running'
    

def infoHandler(sock):
  global process
  
  if process != None:
    print '   SHOW-INFO'
    
    print '   Sending z to omxplayer ...'
    #for i in process.stdout.readline(): sock.send(i)
    process.stdin.write('z')
    for i in process.stdout.readline(): sock.send(i)
    
    process.stdin.flush()
    
    sock.send("  SHOW-INFO")
  else:
    print '    SHOW-INFO-' + mode + ' ignored. Omxplayer NOT Running'

    
def atrackHandler(sock, mode):
  global process
  
  if process != None:
    print '   ATRACK-' + mode  
    
    if mode == "UP": 
      print '   Sending k to omxplayer ...'
      process.stdin.write('k')
    elif mode == "DOWN":
      print '   Sending j to omxplayer ...'
      process.stdin.write('j')
    
    process.stdin.flush()
    
    sock.send("  ATRACK-" + mode)
  else:
    print '    ATRACK-' + mode + ' ignored. Omxplayer NOT Running'
    

def subtToggHandler(sock, mode):
  global process
  
  if process != None:
    print '   STRACK-' + mode  
    
    if mode == "UP": 
      print '   Sending m to omxplayer ...'
      process.stdin.write('m')
    elif mode == "DOWN":
      print '   Sending n to omxplayer ...'
      process.stdin.write('n')
    
    process.stdin.flush()
    
    sock.send("  STRACK-" + mode)
  else:
    print '    STRACK-' + mode + ' ignored. Omxplayer NOT Running'



def subtToggHandler(sock, mode):
  global process
  
  if process != None:
    print '   SUBT-TOGG-' + mode  
    
    print '   Sending s to omxplayer ...'
    process.stdin.write('s')
    process.stdin.flush()
    
    sock.send("  SUBT-TOGG-" + mode)
  else:
    print '    SUBT-TOGG-' + mode + ' ignored. Omxplayer NOT Running'


    
def seekHandler(sock, mode):
  global process
  
  if process != None:
    print '   SEEK-' + mode  
    
    if mode == "FW":
      print '   Sending RIGHT-ARROW to omxplayer ...'
      process.stdin.write('\e[C')                               # Es codificacion interna de caracteres de Python?
    elif mode == "BW":
      print '   Sending LEFT-ARROW to omxplayer ...'
      process.stdin.write('\e[D')
    elif mode == "UP":
      print '   Sending UP-ARROW to omxplayer ...'
      process.stdin.write('\e[A')
    elif mode == "DOWN":
      print '   Sending DOWN-ARROW to omxplayer ...'
      process.stdin.write('\e[B')
    
    process.stdin.flush()
    
    sock.send("  SEEK-" + mode)
  else:
    print '    SEEK-' + mode + ' ignored. Omxplayer NOT Running'


def getTracksHandler(sock):
  global trackVideo
  
  print '   GET-TRACKS'
  
  if trackVideo != None:
    print '      Current Video Track : ' + trackVideo
    sock.send(trackVideo)
  else:
    print '      Current Video Track : NO-TRACKS'
    sock.send('NO-TRACKS')
  
  
def killOmxplayer():
  global process
  
  if process != None:
    
    # Otras teclas dejan la pipe tostada. Sin embargo la 'q' la recibe correctamente y finaliza. ???
    try:
      print 'Terminating omxplayer. Sending q.'
      #process.communicate('q')
      process.stdin.write('q')
      process.stdin.flush()
    except:  
      print '\n\n>>> traceback <<<'
      traceback.print_exc(file=sys.stdout)
      print '>>> end of track <<<\n\n'
    
    try:
      if process != None:
        #if  process.poll():
        #if process.is_alive():
        print 'Killing python subprocess.'
        process.kill()
    except:  
      printException()
  
  trackVideo    = None
  process       = None
  
    

## Main
signal.signal(signal.SIGTERM, handleSigTERM)   
signal.signal(signal.SIGINT,  handleSigTERM)

config = {}
execfile("bbOmxControlServer.conf", config)

prompt          = "BB-OMXPLAYER>"
badUsage        = "Bad Usage."
errorInRequest  = "Error managing Request."
usageMsg        = "\n\
----------------------------------------------\n\
- Banpberry OMXPLAYER Control Server \n\
-   Banshee 2013 \n\
----------------------------------------------\n\
 Usage :\n \
  PLAY <uri>\n \
  PAUSE\n \
  VOL-UP\n \
  VOL-DOWN\n \
  SEEK-FW\n \
  SEEK-BW\n \
  SEEK-UP\n \
  SEEK-DOWN\n \
  SPEED-UP\n \
  SPEED-DOWN\n \
  REWIND\n \
  FAST-FW\n \
  SHOW-INFO\n \
  ATRACK-UP\n \
  ATRACK-DOWN\n \
  STRACK-UP\n \
  STRACK-DOWN\n \
  SUBT-TOGG\n \
  GET-TRACKS\n \
  CLOSE\n\n\
  Server automaticaly close connections. No session protocol.\n\
"

process         = None
trackVideo      = None

if len(sys.argv) != 2:
  print "badUsage. bbOmxControlServer <port>"
  sys.exit(0)

port = int(sys.argv[1])

print '----------------------------------------------'
print '- Banpberry OMXPLAYER Control Server '
print '-   Banshee 2013 '
print '-'
print '- ' +str(datetime.datetime.now())
print '----------------------------------------------'
print 
print 'Creating server at port :' + str(port) + ' ...'

# Creacion del socket
s = socket.socket()
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Creacion de puerto de conexiones
#host = "192.168.15.1"
#host = config["ip"]
s.bind(('', port))          # Bind to the port at any address. Same as 0.0.0.0
s.listen(5)                 # Now wait for client connection.
c = None

# Bucle de escucha de conexiones
while True:

  print "\n<" + str(datetime.datetime.now()) + "> Waiting for new connection ... "
  sys.stdout.flush()
  sys.stderr.flush()
  c, addr = s.accept()     # Establish connection with client.
  
  try:
    print "<" + str(datetime.datetime.now()) + "> Got connection from ", addr
    c.send(usageMsg)
    c.send(prompt)
    
    
    if process != None:
      print ' omxplayer IS Running'
    else:
      print ' omxplayer IS NOT Running'
    
    # Espera sobre el socket para lectura de peticion
    print "<" + str(datetime.datetime.now()) + ">  Waiting for data ... "
    data = c.recv(1024)
    

    if data: 
      #print "RAW Data '" + data + "'"
      data = data.strip('\n\t\r')
      
      print "   Striped Data '" + data + "'"
      c.send(" Received Data : '" + data + "'\n")
      
      # Parseo de respuesta y ejecucion de operacion
      reqArgs = data.split(' ')
      
      if len(reqArgs) >= 1:
        if reqArgs[0] == 'NONE':
          print 'Ignoring command.'
        if reqArgs[0] == 'PLAY':
          playHandler(c, reqArgs[1])
        elif reqArgs[0] == 'STOP':
          print '  STOP not implemented yet'
          c.send('STOP not implemented yet\n')
        elif reqArgs[0] == "PAUSE":
          pauseHandler(c)
        elif reqArgs[0] == "CLOSE":
          closeHandler(c)
        elif reqArgs[0] == "VOL-UP":
          volumeHandler(c, 'UP')
        elif reqArgs[0] == "VOL-DOWN":
          volumeHandler(c, 'DOWN')
        elif reqArgs[0] == "SEEK-FW":
          seekHandler(c, 'FW')
        elif reqArgs[0] == "SEEK-BW":
          seekHandler(c, 'BW')
        elif reqArgs[0] == "SEEK-UP":
          seekHandler(c, 'UP')
        elif reqArgs[0] == "SEEK-DOWN":
          seekHandler(c, 'DOWN')
        elif reqArgs[0] == "SPEED-UP":
          speedHandler(c, 'UP')
        elif reqArgs[0] == "SPEED-DOWN":
          speedHandler(c, 'DOWN')
        elif reqArgs[0] == "REWIND":
          motionHandler(c, 'R')
        elif reqArgs[0] == "FAST-FW":
          motionHandler(c, 'F')
        elif reqArgs[0] == "SHOW-INFO":
          infoHandler(c)
        elif reqArgs[0] == "ATRACK-UP":
          atrackHandler(c, 'UP')
        elif reqArgs[0] == "ATRACK-DOWN":
          atrackHandler(c, 'DOWN')
        elif reqArgs[0] == "STRACK-UP":
          subHandler(c, 'UP')
        elif reqArgs[0] == "STRACK-DOWN":
          subHandler(c, 'DOWN')
        elif reqArgs[0] == "SUBT-TOGG":
          subtToggHandler(c)
        elif reqArgs[0] == "GET-TRACKS":
          getTracksHandler(c)
        else:
          print "  UNKNOWN Command : '" + reqArgs[0] + "'"
          c.send("UNKNOWN Command : '" + reqArgs[0] + "'\n")
        
    else:
      c.send(badUsage);
    
    closeConnection()  
  except:
    print errorInRequest
    
    printException()
    
    closeConnection()
    #raise             # Test Mode
