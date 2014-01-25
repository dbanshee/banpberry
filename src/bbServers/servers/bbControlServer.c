/*
 * bbControlServer.c
 * 
 * Banpberry Control Server - Banshee 2014
 * 
 *      mail: dbanshee@gmail.com
 * 
 * Servidor de control de servicios general.
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#include "../libs/serversocket.h"
#include "../libs/logger.h"
#include "../libs/procmanager.h"




// Signal Handler
void signalHandler(int sigNum);

// Manejador de peticion de conexion
int controlHandler(int connfd);



const char requestHeader [1024] = 
"-------------------\n\
- bbControlServer -\n\
-------------------\n\
Usage :\n\n\
  START <service>\n\
  STOP <service>\n\
  RESTART <service>\n\
BBCONTROL>";


// Global vars
serverSocketContext serverCtx;

// Main Control Server
int main(int argc, char* argv){
    
    // Signal Handlers
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    
    
    printf("-------------------------------\n");
    printf("-- Banpberry Control Server    \n");
    printf("--     Banshee 2014            \n");
    printf("-- %s\n", getCurrentDate());
    printf("-------------------------------\n\n\n");
    
    // Inicializacion del contexto del servidor
    memset(&serverCtx, 0, sizeof(serverSocketContext));
    serverCtx.serverPort       = 9000;                  // Puerto de conexion
    serverCtx.serverLoop       = 1;                     // Condicion de terminacion del bucle
    serverCtx.requestHandler   = &controlHandler;       // Callback de tratamiento de conexion
    
    // Bucle de escucha de conexiones
    socketServerLoop(&serverCtx);
        
    return 0;
}

void signalHandler(int sigNum){
    
    if(sigNum == SIGTERM)
        blog(LOG_INFO, "Signal SIGTERM received");
    else if(sigNum == SIGINT)
        blog(LOG_INFO, "Signal SIGINT received");
    else
        return;
    
    finishServer(&serverCtx);
}



/*
 * Manejador de peticion de conexion
 */
int controlHandler(int connfd){
    size_t lineSize = 1024;
    ssize_t nread;
    char line [lineSize];
    
    blog(LOG_INFO, "Handling request on Control Server.");
    
    // Write header & prompt
    serverWriteBuffer(connfd, (char *) requestHeader, strlen(requestHeader));
    
    // Read line from socket
    blog(LOG_INFO, "Waiting client data ...");
    nread = serverReadBuffer(connfd, line, lineSize);
    
    if(nread == -1){
        blog(LOG_ERROR, "Error reading from client.");
    }else if(nread == 0){
        serverWriteBuffer(connfd, REQUEST_ERROR, strlen(REQUEST_ERROR));
    }else{
        /* 
         * Process Request
         *  ... 
         */
        serverWriteBuffer(connfd, REQUEST_OK, strlen(REQUEST_OK));
    }
    
    return 0;
}

