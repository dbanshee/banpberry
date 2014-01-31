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

#include "../libs/stringutils.h"
#include "../libs/serversocket.h"
#include "../libs/logger.h"
#include "../libs/procmanager.h"

#define START_COMMAND           "START"
#define STOP_COMMAND            "STOP"
#define RESTART_COMMAND         "RESTART"
#define FORCE                   "FORCE"

#define OMXPLAYER_SERVICE       "OMXPLAYER"
#define VLC_SERVICE             "VLC"
#define MOPIDY_SERVICE          "MOPIDY"
#define APACHE_SERVICE          "APACHE"
#define FTP_SERVICE             "FTP"



// Signal Handler
void signalHandler(int sigNum);

// Manejador de peticion de conexion
int controlHandler(int connfd);

// Control Services Handlers
int omxplayerServiceHandler(char **args, int nargs);
int vlcServiceHandler(char **args, int nargs);
int mopidyServiceHandler(char **args, int nargs);
int apacheServiceHandler(char **args, int nargs);
int ftpServiceHandler(char **args, int nargs);


const char requestHeader [1024] = 
"-------------------\n\
- bbControlServer -\n\
-------------------\n\
Usage :\n\n\
  START <service>\n\
  STOP <service> [FORCE]\n\
  RESTART <service> [FORCE]\n\
\
     service = OMXPLAYER|VLC|MOPIDY|APACHE|FTP\n\
BBCONTROL>";


// Global vars
serverSocketContext serverCtx;

// Main Control Server
int main(int argc, char *argv){
    
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
   
        // Process request
        int requestError = 0;
        
        char** spl = splitLine(line, " \n");
        int nargs  = 0;
        while(spl[nargs] != NULL) nargs++;
        
        if(nargs >= 2 && nargs <= 3){
            
            if((strcasecmp(START_COMMAND,  spl[0]) == 0 && nargs == 2) || 
               ((strcasecmp(STOP_COMMAND,  spl[0]) == 0 || strcasecmp(RESTART_COMMAND, spl[0]) == 0) && 
                (nargs == 2 || strcasecmp(spl[2], FORCE) == 0))){
                
                if(strcasecmp(OMXPLAYER_SERVICE, spl[1]) == 0)
                    requestError = omxplayerServiceHandler(spl, nargs);
                else if(strcasecmp(VLC_SERVICE, spl[1]) == 0)
                    requestError = vlcServiceHandler(spl, nargs);
                else if(strcasecmp(MOPIDY_SERVICE, spl[1]) == 0)
                    requestError = mopidyServiceHandler(spl, nargs);
                else if(strcasecmp(APACHE_SERVICE, spl[1]) == 0)
                    requestError = apacheServiceHandler(spl, nargs);
                else if(strcasecmp(FTP_SERVICE, spl[1]) == 0)
                    requestError = ftpServiceHandler(spl, nargs);
                else
                    requestError = -1;
            }else{
                serverWriteBuffer(connfd, REQUEST_ERROR, strlen(REQUEST_ERROR));
                requestError = -1;
            }
        }
        
        free(spl);
        
        if(requestError == -1)
            serverWriteBuffer(connfd, REQUEST_ERROR, strlen(REQUEST_ERROR));
        else
            serverWriteBuffer(connfd, REQUEST_OK, strlen(REQUEST_OK)); 
    }
    
    return 0;
}


int genericScriptServiceHandler(){

}

int omxplayerServiceHandler(char **args, int nargs){

}

int vlcServiceHandler(char **args, int nargs){

}

int mopidyServiceHandler(char **args, int nargs){
    processContext procCtx;
    initializeProcessContex(&procCtx);
    
    
    procCtx.binPath    = "/bin/bash";
    procCtx.args       = (char**) malloc(sizeof(char*)*5);
    
    procCtx.args[0] = "/bin/bash";
    procCtx.args[1] = "/bb/scripts/bbServices/bbMopidy.sh";
    procCtx.args[2] = args[0];
    
    if(nargs == 3){
        procCtx.args[3] = args[2]; // Opcion FORCE
        procCtx.args[4] = NULL;
    }else
        procCtx.args[3] = NULL;
    
    createProcess(&procCtx);
    
    if(procCtx.status == RUNNING)
        waitProcess(&procCtx);
    else
        blog(LOG_ERROR, "Error executing Mopidy Service");
    
    free(procCtx.args);
}

int apacheServiceHandler(char **args, int nargs){

}

int ftpServiceHandler(char **args, int nargs){

}
