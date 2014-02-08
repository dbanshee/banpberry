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
#include <sys/stat.h>

#include "../libs/stringutils.h"
#include "../libs/serversocket.h"
#include "../libs/logger.h"
#include "../libs/procmanager.h"

// Standard options
#define START_COMMAND           "start"
#define STOP_COMMAND            "stop"
#define RESTART_COMMAND         "restart"
#define FORCE_OPTION            "force"

// Service states
#define SERVICE_ON              "ON"
#define SERVICE_OFF             "OFF"

// Services
#define NSERVICES               9               // No count GET_ALL_SERVICE_STATUS
#define APACHE_SERVICE          "APACHE"
#define DHCP_SERVICE            "DHCP"
#define DNS_SERVICE             "DNS"
#define FTP_SERVICE             "FTP"
#define MOPIDY_SERVICE          "MOPIDY"
#define STARTX_SERVICE          "STARTX"
#define OMXPLAYER_SERVICE       "OMXPLAYER"
#define VLC_SERVICE             "VLC"
#define VNC_SERVICE             "VNC"

#define GET_ALL_SERVICE_STATUS  "GETALLSERVICESTATUS"

// SO Executables
#define NEXECS                  10
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

// Process Names
#define NPROCS                  9
#define APACHE_PROC_NAME        "apache2"
#define DHCP_PROC_NAME          "dhcpd"
#define DNS_PROC_NAME           "bind9"
#define FTP_PROC_NAME           "vsftpd"
#define MOPIDY_PROC_NAME        "mopidy"
#define OMXPLAYER_PROC_NAME     "omxplayer.bin"
#define STARTX_PROC_NAME        "startx"
#define VLC_PROC_NAME           "vlc"
#define VNC_PROC_NAME           "x11vnc"

// General
#define MAXBUFFSIZE             1024


// Signal Handler
void signalHandler(int sigNum);

// Init Checks
int checkServicesExecutables();

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
int getAllServiceStatusHandler(char **args, int nargs, int connfd);


static int genericBBScriptHandler(char * service, char * command, char** args, int nargs);



const char *soExecs   [NEXECS]    = { BASH_EXEC,   APACHE_EXEC,    DHCP_EXEC,   DNS_EXEC, FTP_EXEC, 
                                      MOPIDY_EXEC, OMXPLAYER_EXEC, STARTX_EXEC, VLC_EXEC, VNC_EXEC };

const char *servNames [NSERVICES] = { APACHE_SERVICE,    DHCP_SERVICE,   DNS_SERVICE, FTP_SERVICE, MOPIDY_SERVICE,
                                      OMXPLAYER_SERVICE, STARTX_SERVICE, VLC_SERVICE, VNC_SERVICE };

const char *procNames [NPROCS]    = { APACHE_PROC_NAME,    DHCP_PROC_NAME,   DNS_PROC_NAME, FTP_PROC_NAME, MOPIDY_PROC_NAME,
                                      OMXPLAYER_PROC_NAME, STARTX_PROC_NAME, VLC_PROC_NAME, VNC_PROC_NAME };


const char requestHeader [1024] = 
"-------------------\n\
- bbControlServer -\n\
-------------------\n\
Usage :\n\n\
  APACHE <start|stop|restart> [force]\n\
  DHCP <start|stop|restart> [force]\n\
  DNS <start|stop|restart> [force]\n\
  FTP <start|stop|restart> [force]\n\
  MOPIDY <start|stop|restart> [force]\n\
  STARTX <start|stop|restart> [force]\n\
  OMXPLAYER stop [force]\n\
  VLC stop [force]\n\
  VNC <start|stop|restart> [force]\n\
\n\
  GETALLSERVICESTATUS\n\
BBCONTROL>";


// Global vars
serverSocketContext serverCtx;

// Main Control Server
int main(int argc, char *argv){
    
    //temp();
    
    // Signal Handlers
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    
    
    printf("-----------------------------------------\n");
    printf("-- Banpberry Control Server    \n");
    printf("--     Banshee 2014            \n");
    printf("-- Start at : %s\n", getCurrentDate());
    printf("-----------------------------------------\n\n\n");
    
    
    checkServicesExecutables();
    
    // Inicializacion del contexto del servidor
    memset(&serverCtx, 0, sizeof(serverSocketContext));
    serverCtx.serverPort       = 9000;                  // Puerto de conexion
    serverCtx.serverLoop       = 1;                     // Condicion de terminacion del bucle
    serverCtx.requestHandler   = &controlHandler;       // Callback de tratamiento de conexion
    
    // Bucle de escucha de conexiones
    socketServerLoop(&serverCtx);
        
    return 0;
}

// SO Signal handlers
void signalHandler(int sigNum){
    
    if(sigNum == SIGTERM)
        blog(LOG_INFO, "Signal SIGTERM received");
    else if(sigNum == SIGINT)
        blog(LOG_INFO, "Signal SIGINT received");
    else
        return;
    
    finishServer(&serverCtx);
}

// Check external binaries and scripts.
int checkServicesExecutables(){
    int i;
    struct stat fstat;
    int existsAll = 0;
    
    for(i = 0; i < NEXECS; i++)
        if(stat(soExecs[i], &fstat) == -1){
            blog(LOG_WARN, "Banpberry Control Service required file '%s' not found.", soExecs[i]);
            existsAll = -1;
        }
    
    return existsAll;
}



/*
 * Conexion Request Handler.
 * 
 * Invoqued by socketServerLoop.
 */
int controlHandler(int connfd){
    ssize_t nread;
    char line [MAXBUFFSIZE];
    
    blog(LOG_INFO, "Handling request on Control Server.");
    
    // Write header & prompt
    serverWriteBuffer(connfd, (char *) requestHeader, strlen(requestHeader));
    
    // Read line from socket
    blog(LOG_INFO, "Waiting client data ...");
    nread = serverReadBuffer(connfd, line, MAXBUFFSIZE-1);
    
    if(nread == -1)
        blog(LOG_ERROR, "Error reading from client.");
    else if(nread == 0)
        serverWriteBuffer(connfd, REQUEST_ERROR, strlen(REQUEST_ERROR));
    else{
   
        // Process request
        int requestError = 0;
        
        line[nread] = '\0';   
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
            else if(strcasecmp(spl[0], GET_ALL_SERVICE_STATUS) == 0)
                requestError = getAllServiceStatusHandler(spl, nargs, connfd);
            else
                requestError = -1;
        }
        
        free(spl);
        
        if(requestError == -1)
            serverWriteBuffer(connfd, REQUEST_ERROR, strlen(REQUEST_ERROR));
        else
            serverWriteBuffer(connfd, REQUEST_OK, strlen(REQUEST_OK)); 
    }
    
    return 0;
}

/*
 * Generic handler. Handles request like <service> <start|stop|restart>.
 * 
 *      Run and wait for /bin/bash <servicebbScript>.sh  <start|stop|restart>
 */
static int genericBBScriptHandler(char *service, char *command, char** args, int nargs){
    int requestOk = 1;
    
    blog(LOG_INFO, "%s Service action requested", service);
    
    processContext *procCtx;
    
    if((nargs < 2) || (nargs > 3) ||
       (nargs >= 2 && (strcasecmp(args[1], START_COMMAND)    != 0 &&  
                       strcasecmp(args[1], STOP_COMMAND)     != 0 && 
                       strcasecmp(args[1], RESTART_COMMAND)  != 0 )) || 
       (nargs == 3 &&  strcasecmp(args[2], FORCE_OPTION)     != 0)   ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    
    if(nargs == 3)
        procCtx = createProcessContext(4, BASH_EXEC, command, args[1], args[2]);
    else
        procCtx = createProcessContext(3, BASH_EXEC, command, args[1]);
    
    createProcess(procCtx);
    
    if(procCtx->status == RUNNING)
        waitProcess(procCtx);
    
    if(procCtx->status != FINISHED){
        blog(LOG_ERROR, "Error executing %s Service", service);
        requestOk = -1;
    }
    
    
    freeProcessContext(procCtx);
    
    return requestOk;
}

int apacheServiceHandler(char **args, int nargs){
    return genericBBScriptHandler(APACHE_SERVICE, APACHE_EXEC, args, nargs);
}

int ftpServiceHandler(char **args, int nargs){
    return genericBBScriptHandler(FTP_SERVICE, FTP_EXEC, args, nargs);
}

int dhcpServiceHandler(char **args, int nargs){
    return genericBBScriptHandler(DHCP_SERVICE, DHCP_EXEC, args, nargs);
}

int dnsServiceHandler(char **args, int nargs){
    return genericBBScriptHandler(DNS_SERVICE, DNS_EXEC, args, nargs);
}

int mopidyServiceHandler(char **args, int nargs){
    return genericBBScriptHandler(MOPIDY_SERVICE, MOPIDY_EXEC, args, nargs);
}

int startXServiceHandler(char **args, int nargs){
    return genericBBScriptHandler(STARTX_SERVICE, STARTX_EXEC, args, nargs);
}

int omxplayerServiceHandler(char **args, int nargs){
    return genericBBScriptHandler(OMXPLAYER_SERVICE, OMXPLAYER_EXEC, args, nargs);
}

int vlcServiceHandler(char **args, int nargs){
    return genericBBScriptHandler(VLC_SERVICE, VLC_EXEC, args, nargs);
}

int vncServiceHandler(char **args, int nargs){
    return genericBBScriptHandler(VNC_SERVICE, VNC_EXEC, args, nargs);
}

int getAllServiceStatusHandler(char **args, int nargs, int connfd){
    int i;
    char buff[MAXBUFFSIZE];
    
    blog(LOG_INFO, "Service Get All Status requested");
    
    if(nargs != 1){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
    
    for(i = 0; i < NPROCS; i++){
        if(existsProcess(procNames[i]) == 1) // Batch alternative check process function?
            snprintf(buff, MAXBUFFSIZE, "%s %s\n", servNames[i], SERVICE_ON);
        else
            snprintf(buff, MAXBUFFSIZE, "%s %s\n", servNames[i], SERVICE_OFF);
            
        serverWriteBuffer(connfd, buff, strlen(buff));
    }
    
    return 1;
}
