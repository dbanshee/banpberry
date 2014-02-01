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

// Standard options
#define START_COMMAND           "start"
#define STOP_COMMAND            "stop"
#define RESTART_COMMAND         "restart"
#define FORCE_OPTION            "force"

// Services
#define APACHE_SERVICE          "APACHE"
#define DHCP_SERVICE            "DHCP"
#define DNS_SERVICE             "DNS"
#define FTP_SERVICE             "FTP"
#define MOPIDY_SERVICE          "MOPIDY"
#define STARTX_SERVICE          "STARTX"
#define OMXPLAYER_SERVICE       "OMXPLAYER"
#define VLC_SERVICE             "VLC"
#define VNC_SERVICE             "VNC"

// SO Executables
#define BASH_EXEC               "/bin/bash"
#define APACHE_EXEC             "/bb/scripts/bbServices/bbApache.sh"
#define DHCP_EXEC               "/bb/scripts/bbServices/bbDhcp.sh"
#define DNS_EXEC                "/bb/scripts/bbServices/bbDns.sh"
#define FTP_EXEC                "/bb/scripts/bbServices/bbFtp.sh"
#define MOPIDY_EXEC             "/bb/scripts/bbServices/bbMopidy.sh"
#define OMXPLAYER_EXEC          "/bb/scripts/bbServices/bbOmxplayer.sh"
#define STARTX_EXEC             "/bb/scripts/bbServices/bbStartX.sh"
#define VLC_EXEC                "/bb/scripts/bbServices/bbVlc.sh"
#define VNC_EXEC                "/bb/scripts/bbServices/bbVnc.sh"



// Signal Handler
void signalHandler(int sigNum);

// Manejador de peticion de conexion
int controlHandler(int connfd);

// Control Services Handlers
int apacheServiceHandler(char **args, int nargs);
int dhcpServiceHandler(char **args, int nargs);
int dnsServiceHandler(char **args, int nargs);
int ftpServiceHandler(char **args, int nargs);
int mopidyServiceHandler(char **args, int nargs);
int startXServiceHandler(char **args, int nargs);
int omxplayerServiceHandler(char **args, int nargs);
int startXServiceHandler(char **args, int nargs);
int vlcServiceHandler(char **args, int nargs);
int vncServiceHandler(char **args, int nargs);


void temp();

const char requestHeader [1024] = 
"-------------------\n\
- bbControlServer -\n\
-------------------\n\
Usage :\n\n\
  APACHE <[start|stop|restart]> [force]\n\
  DHCP <[start|stop|restart]> [force]\n\
  DNS <[start|stop|restart]> [force]\n\
  FTP <[start|stop|restart]> [force]\n\
  MOPIDY <[start|stop|restart]> [force]\n\
  STARTX <[start|stop|restart]> [force]\n\
  OMXPLAYER <[start|stop|restart]> [force]\n\
  VLC <start|stop|restart> [force]\n\
  VNC [start|stop|restart]\n\
BBCONTROL>";


// Global vars
serverSocketContext serverCtx;

// Main Control Server
int main(int argc, char *argv){
    
    // Signal Handlers
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    
    
    printf("-----------------------------------------\n");
    printf("-- Banpberry Control Server    \n");
    printf("--     Banshee 2014            \n");
    printf("-- Start at :%s\n", getCurrentDate());
    printf("-----------------------------------------\n\n\n");
    
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
        
        
        if(nargs >= 1){
            if(strcasecmp(spl[0], APACHE_SERVICE) == 0)
                requestError = apacheServiceHandler(spl, nargs);
            else if(strcasecmp(spl[0], DHCP_SERVICE) == 0)
                requestError = dhcpServiceHandler(spl, nargs);
            else if(strcasecmp(spl[0], DNS_SERVICE) == 0)
                requestError = dnsServiceHandler(spl, nargs);
            else if(strcasecmp(spl[0], FTP_SERVICE) == 0)
                requestError = ftpServiceHandler(spl, nargs);
            else if(strcasecmp(spl[0], MOPIDY_SERVICE) == 0)
                requestError = mopidyServiceHandler(spl, nargs);
            else if(strcasecmp(spl[0], STARTX_SERVICE) == 0)
                requestError = startXServiceHandler(spl, nargs);
            else if(strcasecmp(spl[0], OMXPLAYER_SERVICE) == 0)
                requestError = omxplayerServiceHandler(spl, nargs);
            else if(strcasecmp(spl[0], VLC_SERVICE) == 0)
                requestError = vlcServiceHandler(spl, nargs);
            else if(strcasecmp(spl[0], VNC_SERVICE) == 0)
                requestError = vncServiceHandler(spl, nargs);
            else{
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

int apacheServiceHandler(char **args, int nargs){
    int requestOk = 1;
    
    blog(LOG_INFO, "Apache Service action requested");
    
    processContext procCtx;
    initializeProcessContex(&procCtx);
    
    if((nargs < 2) || (nargs > 3) ||
       (nargs >= 2 && (strcasecmp(args[1], START_COMMAND)    != 0 &&  
                       strcasecmp(args[1], STOP_COMMAND)     != 0 && 
                       strcasecmp(args[1], RESTART_COMMAND)  != 0 )) || 
       (nargs == 3 &&  strcasecmp(args[2], FORCE_OPTION)     != 0)   ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    procCtx.binPath     = BASH_EXEC;
    procCtx.args        = (char**) malloc(sizeof(char*)*5);
    
    procCtx.args[0]     = BASH_EXEC;
    procCtx.args[1]     = APACHE_EXEC;
    procCtx.args[2]     = args[1];
    
    if(nargs == 3){
        procCtx.args[3] = args[2]; // Opcion FORCE
        procCtx.args[4] = NULL;
    }else
        procCtx.args[3] = NULL;
    
    createProcess(&procCtx);
    
    if(procCtx.status == RUNNING)
        waitProcess(&procCtx);
    
    if(procCtx.status != FINISHED){
        blog(LOG_ERROR, "Error executing Apache Service");
        requestOk = -1;
    }
    
    free(procCtx.args);
    
    return requestOk;
}


int ftpServiceHandler(char **args, int nargs){
    int requestOk = 1;
    
    blog(LOG_INFO, "Ftp Service action requested");
    
    processContext procCtx;
    initializeProcessContex(&procCtx);
    
    if((nargs < 2) || (nargs > 3) ||
       (nargs >= 2 && (strcasecmp(args[1], START_COMMAND)    != 0 &&  
                       strcasecmp(args[1], STOP_COMMAND)     != 0 && 
                       strcasecmp(args[1], RESTART_COMMAND)  != 0 )) || 
       (nargs == 3 &&  strcasecmp(args[2], FORCE_OPTION)     != 0)   ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    procCtx.binPath     = BASH_EXEC;
    procCtx.args        = (char**) malloc(sizeof(char*)*5);
    
    procCtx.args[0]     = BASH_EXEC;
    procCtx.args[1]     = FTP_EXEC;
    procCtx.args[2]     = args[1];
    
    if(nargs == 3){
        procCtx.args[3] = args[2]; // Opcion FORCE
        procCtx.args[4] = NULL;
    }else
        procCtx.args[3] = NULL;
    
    createProcess(&procCtx);
    
    if(procCtx.status == RUNNING)
        waitProcess(&procCtx);
    
    if(procCtx.status != FINISHED){
        blog(LOG_ERROR, "Error executing Ftp Service");
        requestOk = -1;
    }
    
    free(procCtx.args);
    
    return requestOk;
}


int dhcpServiceHandler(char **args, int nargs){
    int requestOk = 1;
    
    blog(LOG_INFO, "Dhcp Service action requested");
    
    processContext procCtx;
    initializeProcessContex(&procCtx);
    
    if((nargs < 2) || (nargs > 3) ||
       (nargs >= 2 && (strcasecmp(args[1], START_COMMAND)    != 0 &&  
                       strcasecmp(args[1], STOP_COMMAND)     != 0 && 
                       strcasecmp(args[1], RESTART_COMMAND)  != 0 )) || 
       (nargs == 3 &&  strcasecmp(args[2], FORCE_OPTION)     != 0)   ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    procCtx.binPath     = BASH_EXEC;
    procCtx.args        = (char**) malloc(sizeof(char*)*5);
    
    procCtx.args[0]     = BASH_EXEC;
    procCtx.args[1]     = DHCP_EXEC;
    procCtx.args[2]     = args[1];
    
    if(nargs == 3){
        procCtx.args[3] = args[2]; // Opcion FORCE
        procCtx.args[4] = NULL;
    }else
        procCtx.args[3] = NULL;
    
    createProcess(&procCtx);
    
    if(procCtx.status == RUNNING)
        waitProcess(&procCtx);
    
    if(procCtx.status != FINISHED){
        blog(LOG_ERROR, "Error executing Dhcp Service");
        requestOk = -1;
    }
    
    free(procCtx.args);
    
    return requestOk;
}


int dnsServiceHandler(char **args, int nargs){
    int requestOk = 1;
    
    blog(LOG_INFO, "Dns Service action requested");
    
    processContext procCtx;
    initializeProcessContex(&procCtx);
    
    if((nargs < 2) || (nargs > 3) ||
       (nargs >= 2 && (strcasecmp(args[1], START_COMMAND)    != 0 &&  
                       strcasecmp(args[1], STOP_COMMAND)     != 0 && 
                       strcasecmp(args[1], RESTART_COMMAND)  != 0 )) || 
       (nargs == 3 &&  strcasecmp(args[2], FORCE_OPTION)     != 0)   ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    procCtx.binPath     = BASH_EXEC;
    procCtx.args        = (char**) malloc(sizeof(char*)*5);
    
    procCtx.args[0]     = BASH_EXEC;
    procCtx.args[1]     = DNS_EXEC;
    procCtx.args[2]     = args[1];
    
    if(nargs == 3){
        procCtx.args[3] = args[2]; // Opcion FORCE
        procCtx.args[4] = NULL;
    }else
        procCtx.args[3] = NULL;
    
    createProcess(&procCtx);
    
    if(procCtx.status == RUNNING)
        waitProcess(&procCtx);
    
    if(procCtx.status != FINISHED){
        blog(LOG_ERROR, "Error executing Dns Service");
        requestOk = -1;
    }
    
    free(procCtx.args);
    
    return requestOk;
}

int mopidyServiceHandler(char **args, int nargs){
    int requestOk = 1;
    
    blog(LOG_INFO, "Mopidy Service action requested");
    
    processContext procCtx;
    initializeProcessContex(&procCtx);
    
    if((nargs < 2) || (nargs > 3) ||
       (nargs >= 2 && (strcasecmp(args[1], START_COMMAND)    != 0 &&  
                       strcasecmp(args[1], STOP_COMMAND)     != 0 && 
                       strcasecmp(args[1], RESTART_COMMAND)  != 0 )) || 
       (nargs == 3 &&  strcasecmp(args[2], FORCE_OPTION)     != 0)   ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    procCtx.binPath     = BASH_EXEC;
    procCtx.args        = (char**) malloc(sizeof(char*)*5);
    
    procCtx.args[0]     = BASH_EXEC;
    procCtx.args[1]     = MOPIDY_EXEC;
    procCtx.args[2]     = args[1];
    
    if(nargs == 3){
        procCtx.args[3] = args[2]; // Opcion FORCE
        procCtx.args[4] = NULL;
    }else
        procCtx.args[3] = NULL;
    
    createProcess(&procCtx);
    
    if(procCtx.status == RUNNING)
        waitProcess(&procCtx);
    
    if(procCtx.status != FINISHED){
        blog(LOG_ERROR, "Error executing Mopidy Service");
        requestOk = -1;
    }
    
    free(procCtx.args);
    
    return requestOk;
}

int startXServiceHandler(char **args, int nargs){
    int requestOk = 1;
    
    blog(LOG_INFO, "StartX Service action requested");
    
    processContext procCtx;
    initializeProcessContex(&procCtx);
    
    if((nargs < 2) || (nargs > 3) ||
       (nargs >= 2 && (strcasecmp(args[1], START_COMMAND)    != 0 &&  
                       strcasecmp(args[1], STOP_COMMAND)     != 0 && 
                       strcasecmp(args[1], RESTART_COMMAND)  != 0 )) || 
       (nargs == 3 &&  strcasecmp(args[2], FORCE_OPTION)     != 0)   ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    procCtx.binPath     = BASH_EXEC;
    procCtx.args        = (char**) malloc(sizeof(char*)*5);
    
    procCtx.args[0]     = BASH_EXEC;
    procCtx.args[1]     = STARTX_EXEC;
    procCtx.args[2]     = args[1];
    
    if(nargs == 3){
        procCtx.args[3] = args[2]; // Opcion FORCE
        procCtx.args[4] = NULL;
    }else
        procCtx.args[3] = NULL;
    
    createProcess(&procCtx);
    
    if(procCtx.status == RUNNING)
        waitProcess(&procCtx);
    
    if(procCtx.status != FINISHED){
        blog(LOG_ERROR, "Error executing Mopidy Service");
        requestOk = -1;
    }
    
    free(procCtx.args);
    
    return requestOk;
}

int omxplayerServiceHandler(char **args, int nargs){
    int requestOk = 1;
    
    blog(LOG_INFO, "Omxlplayer Service action requested");
    
    processContext procCtx;
    initializeProcessContex(&procCtx);
    
    if((nargs < 2) || (nargs > 3) ||
       (nargs >= 2 && (strcasecmp(args[1], START_COMMAND)    != 0 &&  
                       strcasecmp(args[1], STOP_COMMAND)     != 0 && 
                       strcasecmp(args[1], RESTART_COMMAND)  != 0 )) || 
       (nargs == 3 &&  strcasecmp(args[2], FORCE_OPTION)     != 0)   ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    procCtx.binPath     = BASH_EXEC;
    procCtx.args        = (char**) malloc(sizeof(char*)*5);
    
    procCtx.args[0]     = BASH_EXEC;
    procCtx.args[1]     = OMXPLAYER_EXEC;
    procCtx.args[2]     = args[1];
    
    if(nargs == 3){
        procCtx.args[3] = args[2]; // Opcion FORCE
        procCtx.args[4] = NULL;
    }else
        procCtx.args[3] = NULL;
    
    createProcess(&procCtx);
    
    if(procCtx.status == RUNNING)
        waitProcess(&procCtx);
    
    if(procCtx.status != FINISHED){
        blog(LOG_ERROR, "Error executing Omxlplayer Service");
        requestOk = -1;
    }
    
    free(procCtx.args);
    
    return requestOk;
}

int vlcServiceHandler(char **args, int nargs){
    int requestOk = 1;
    
    blog(LOG_INFO, "Mopidy Vlc action requested");
    
    processContext procCtx;
    initializeProcessContex(&procCtx);
    
    if((nargs < 2) || (nargs > 3) ||
       (nargs >= 2 && (strcasecmp(args[1], START_COMMAND)    != 0 &&  
                       strcasecmp(args[1], STOP_COMMAND)     != 0 && 
                       strcasecmp(args[1], RESTART_COMMAND)  != 0 )) || 
       (nargs == 3 &&  strcasecmp(args[2], FORCE_OPTION)     != 0)   ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    procCtx.binPath     = BASH_EXEC;
    procCtx.args        = (char**) malloc(sizeof(char*)*5);
    
    procCtx.args[0]     = BASH_EXEC;
    procCtx.args[1]     = VLC_EXEC;
    procCtx.args[2]     = args[1];
    
    if(nargs == 3){
        procCtx.args[3] = args[2]; // Opcion FORCE
        procCtx.args[4] = NULL;
    }else
        procCtx.args[3] = NULL;
    
    createProcess(&procCtx);
    
    if(procCtx.status == RUNNING)
        waitProcess(&procCtx);
    
    if(procCtx.status != FINISHED){
        blog(LOG_ERROR, "Error executing Vlc Service");
        requestOk = -1;
    }
    
    free(procCtx.args);
    
    return requestOk;
}

int vncServiceHandler(char **args, int nargs){
    int requestOk = 1;
    
    blog(LOG_INFO, "Mopidy Vnc action requested");
    
    processContext procCtx;
    initializeProcessContex(&procCtx);
    
    if((nargs < 2) || (nargs > 3) ||
       (nargs >= 2 && (strcasecmp(args[1], START_COMMAND)    != 0 &&  
                       strcasecmp(args[1], STOP_COMMAND)     != 0 && 
                       strcasecmp(args[1], RESTART_COMMAND)  != 0 )) || 
       (nargs == 3 &&  strcasecmp(args[2], FORCE_OPTION)     != 0)   ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    procCtx.binPath     = BASH_EXEC;
    procCtx.args        = (char**) malloc(sizeof(char*)*5);
    
    procCtx.args[0]     = BASH_EXEC;
    procCtx.args[1]     = VNC_EXEC;
    procCtx.args[2]     = args[1];
    
    if(nargs == 3){
        procCtx.args[3] = args[2]; // Opcion FORCE
        procCtx.args[4] = NULL;
    }else
        procCtx.args[3] = NULL;
    
    createProcess(&procCtx);
    
    if(procCtx.status == RUNNING)
        waitProcess(&procCtx);
    
    if(procCtx.status != FINISHED){
        blog(LOG_ERROR, "Error executing Vnc Service");
        requestOk = -1;
    }
    
    free(procCtx.args);
    
    return requestOk;
}
