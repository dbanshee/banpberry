/*
 * bbMediaServer.c
 * 
 * Banpberry Media Server - Banshee 2014
 * 
 *      mail: dbanshee@gmail.com
 * 
 * Servidor de control de reproduccion de media.
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

// Commands
#define PLAYURI_SERVICE         "PLAYURI"
#define PLAY_SERVICE            "PLAY"
#define PAUSE_SERVICE           "PAUSE"
#define STOP_SERVICE            "STOP"
#define NEXT_SERVICE            "NEXT"
#define PREV_SERVICE            "PREV"
#define VOLUP_SERVICE           "VOLUP"
#define VOLDOWN_SERVICE         "VOLDOWN"
#define SEEKF_SERVICE           "SEEKF"
#define SEEKB_SERVICE           "SEEKB"
#define SEEKFF_SERVICE          "SEEKFF"
#define SEEKBB_SERVICE          "SEEKBb"
#define LOOP_SERVICE            "LOOP"
#define FASTER_SERVICE          "FASTER"
#define SLOWER_SERVICE          "SLOWER"
#define INFO_SERVICE            "INFO"
#define ATRACK_SERVICE          "ATRACK"
#define STRACK_SERVICE          "STRACK"
#define GETTRACKS_SERVICE       "GETTRACKS"

// Services
#define NOPTIONS                2
#define VLC_OPTION              "vlc"
#define OMXPLAYER_OPTION        "omx"



// SO Executables
#define NEXECS                  3
#define BASH_EXEC               "/bin/bash"
#define OMXPLAYER_EXEC          "/usr/bin/omxplayer"
#define VLC_EXEC                "/usr/bin/vlc"


#define VLC                     0
#define OMXPLAYER               1

// Vlc specifics commands
#define VLC_PLAY_CMD            "play\n"
#define VLC_PAUSE_CMD           "pause\n"
#define VLC_QUIT_CMD            "quit\n"
#define VLC_NEXT_CMD            "next\n"
#define VLC_PREV_CMD            "prev\n"
#define VLC_VOLUP_CMD           "volup 10\n"
#define VLC_VOLDOWN_CMD         "voldown 10\n"
#define VLC_SEEKF_CMD           "seek 20\n"
#define VLC_SEEKB_CMD           "seek -20\n"
#define VLC_SEEKFF_CMD          "seek 100\n"
#define VLC_SEEKBB_CMD          "seek -100\n"
#define VLC_LOOPON_CMD          "loop on\n"
#define VLC_LOOPOFF_CMD         "loop off\n"
#define VLC_FASTER_CMD          "faster\n"
#define VLC_SLOWER_CMD          "slower\n"
#define VLC_INFO_CMD            "info\n"
#define VLC_ATRACK_CMD          "atrack\n"
#define VLC_STRACK_CMD          "vtrack\n"




// General
#define MAXBUFFSIZE             1024


// Signal Handler
void signalHandler(int sigNum);

// Init Checks
int checkServicesExecutables();

// Manejador de peticion de conexion
int mediaHandler(int connfd);

// Media Services Handlers
static int playUriHandler(char** args, int nargs);
static int playHandler(char** args, int nargs);
static int pauseHandler(char** args, int nargs);
static int nextHandler(char** args, int nargs);
static int prevHandler(char** args, int nargs);
static int volUpHandler(char** args, int nargs);
static int volDownHandler(char** args, int nargs);
static int seekFHandler(char** args, int nargs);
static int seekBHandler(char** args, int nargs);
static int seekFFHandler(char** args, int nargs);
static int seekBBHandler(char** args, int nargs);
static int loopHandler(char** args, int nargs);
static int fasterHandler(char** args, int nargs);
static int slowerHandler(char** args, int nargs);
static int infoHandler(char** args, int nargs);
static int atrackHandler(char** args, int nargs);
static int strackHandler(char** args, int nargs);
static int getTracksHandler(char** args, int nargs);

// General process functions
static processContext*  updateProcessGlobalVar(processContext *procCtx, char* procName);
static int              isProcessRunning(processContext *procCtx);
static int              selectMediaProcess4Uri(char *uri);

// Vlc specific functions
static processContext* openVlc(char* uri);
static void quitVlc(processContext* procCtx);


// Global constants
const char *soExecs   [NEXECS]    = { BASH_EXEC, OMXPLAYER_EXEC, VLC_EXEC };
const char *servNames [NOPTIONS]  = { OMXPLAYER_OPTION, VLC_OPTION };

const char requestHeader [1024] = 
"----------------\n\
- bbMediaServer -\n\
-----------------\n\
Usage :\n\n\
  PLAYURI <uri>\n\
  PLAY <vlc|omx>\n\
  PAUSE <vlc|omx>\n\
  STOP <vlc|omx>\n\
  NEXT <vlc|omx>\n\
  PREV <vlc|omx>\n\
  VOLUP <vlc|omx>\n\
  VOLDOWN <vlc|omx>\n\
  SEEKF <vlc|omx>\n\
  SEEKB <vlc|omx>\n\
  LOOP  <vlc|omx> <on|off>\n\
  FASTER <vlc|omx>\n\
  SLOWER <vlc|omx>\n\
  FFASTER <vlc|omx>\n\
  SSLOWER <vlc|omx>\n\
  INFO <vlc|omx>\n\
  ATRACK <vlc|omx>\n\
  STRACK <vlc|omx>\n\
  GETTRACKS\n\
  BANPORNTV <on|off>\n\
BBMEDIA>";

// Global vars
serverSocketContext serverCtx;
processContext *vlcProcCtx, *omxProcCtx;

// Main Control Server
int main(int argc, char *argv){
    
    //temp();
    
    // Signal Handlers
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    
    
    printf("-----------------------------------------\n");
    printf("-- Banpberry Media Server                \n");
    printf("--     Banshee 2014                      \n");
    printf("-- Start at : %s\n", getCurrentDate());
    printf("-----------------------------------------\n\n\n");
    
    
    checkServicesExecutables();
    
    // Inicializacion del contexto de procesos de media
    vlcProcCtx = NULL;
    omxProcCtx = NULL;
    
    // Inicializacion del contexto del servidor
    memset(&serverCtx, 0, sizeof(serverSocketContext));
    serverCtx.serverPort       = 9001;                  // Puerto de conexion
    serverCtx.serverLoop       = 1;                     // Condicion de terminacion del bucle
    serverCtx.requestHandler   = &mediaHandler;         // Callback de tratamiento de conexion
    
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
            blog(LOG_WARN, "Banpberry Media Server required file '%s' not found.", soExecs[i]);
            existsAll = -1;
        }
    
    return existsAll;
}



/*
 * Conexion Request Handler.
 * 
 * Invoqued by socketServerLoop.
 */
int mediaHandler(int connfd){
    ssize_t nread;
    char line [MAXBUFFSIZE];
    
    blog(LOG_INFO, "Handling request on Media Server.");
    
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
            if(strcasecmp(spl[0], PLAYURI_SERVICE) == 0)
                requestError = playUriHandler(spl, nargs);
            else if(strcasecmp(spl[0], PLAY_SERVICE) == 0)
                requestError = playHandler(spl, nargs);
            else if(strcasecmp(spl[0], PAUSE_SERVICE) == 0)
                requestError = pauseHandler(spl, nargs);
            else if(strcasecmp(spl[0], NEXT_SERVICE) == 0)
                requestError = nextHandler(spl, nargs);
            else if(strcasecmp(spl[0], PREV_SERVICE) == 0)
                requestError = prevHandler(spl, nargs);
            else if(strcasecmp(spl[0], VOLUP_SERVICE) == 0)
                requestError = volUpHandler(spl, nargs);
            else if(strcasecmp(spl[0], VOLDOWN_SERVICE) == 0)
                requestError = volDownHandler(spl, nargs);
            else if(strcasecmp(spl[0], SEEKF_SERVICE) == 0)
                requestError = seekFHandler(spl, nargs);
            else if(strcasecmp(spl[0], SEEKB_SERVICE) == 0)
                requestError = seekBHandler(spl, nargs);
            else if(strcasecmp(spl[0], SEEKFF_SERVICE) == 0)
                requestError = seekFFHandler(spl, nargs);
            else if(strcasecmp(spl[0], SEEKBB_SERVICE) == 0)
                requestError = seekBBHandler(spl, nargs);
            else if(strcasecmp(spl[0], LOOP_SERVICE) == 0)
                requestError = loopHandler(spl, nargs);
            else if(strcasecmp(spl[0], FASTER_SERVICE) == 0)
                requestError = fasterHandler(spl, nargs);
            else if(strcasecmp(spl[0], SLOWER_SERVICE) == 0)
                requestError = slowerHandler(spl, nargs);
            else if(strcasecmp(spl[0], INFO_SERVICE) == 0)
                requestError = infoHandler(spl, nargs);
            else if(strcasecmp(spl[0], ATRACK_SERVICE) == 0)
                requestError = atrackHandler(spl, nargs);
            else if(strcasecmp(spl[0], STRACK_SERVICE) == 0)
                requestError = strackHandler(spl, nargs);
            else if(strcasecmp(spl[0], GETTRACKS_SERVICE) == 0)
                requestError = getTracksHandler(spl, nargs);
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
 * playUriHandler
 *
 */
static int playUriHandler(char** args, int nargs){
    int requestOk = 1;
    int mediaProcess;
    
    blog(LOG_INFO, "Play URI Service requested");
    
    if(nargs != 2){
        blog(LOG_ERROR, "Bad usage.");   
        return -1;
    }
    
    blog(LOG_INFO, "Uri requested : '%s'", args[1]);
    mediaProcess = selectMediaProcess4Uri(args[1]);
        
    if(mediaProcess == VLC){
        
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(vlcProcCtx != NULL){
            blog(LOG_INFO, "Vlc is open. Closing first");
            quitVlc(vlcProcCtx);
            vlcProcCtx = NULL;
        }
        
        vlcProcCtx = openVlc(args[1]);
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);

        if(vlcProcCtx == NULL || vlcProcCtx->status != RUNNING){
            blog(LOG_ERROR, "Vlc cannot be play uri : %s");
            requestOk = -1;
        }
        
    }else if (mediaProcess == OMXPLAYER){
        // TODO
    }else{
        blog(LOG_ERROR, "Unrecognized uri format.");
        requestOk = -1;
    }
    
    return requestOk;
}

static int playHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Play Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_PLAY_CMD, strlen(VLC_PLAY_CMD));
    }else if(omx == 0){
        omxProcCtx = updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int pauseHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Pause Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_PAUSE_CMD, strlen(VLC_PAUSE_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int nextHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Next Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_NEXT_CMD, strlen(VLC_NEXT_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int prevHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Prev Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_PREV_CMD, strlen(VLC_PREV_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}


static int volUpHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Vol Up Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_VOLUP_CMD, strlen(VLC_VOLUP_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}



static int volDownHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Prev Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_VOLDOWN_CMD, strlen(VLC_VOLUP_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}


static int seekFHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "SeekF Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_SEEKF_CMD, strlen(VLC_SEEKF_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int seekBHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "SeekB Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_SEEKB_CMD, strlen(VLC_SEEKB_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int seekFFHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "SeekFF Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_SEEKFF_CMD, strlen(VLC_SEEKFF_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int seekBBHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "SeekBB Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_SEEKBB_CMD, strlen(VLC_SEEKBB_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int loopHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Loop Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_LOOPON_CMD, strlen(VLC_LOOPON_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int fasterHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Faster Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_FASTER_CMD, strlen(VLC_FASTER_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int slowerHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Slower Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_SLOWER_CMD, strlen(VLC_SLOWER_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int infoHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Info Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_INFO_CMD, strlen(VLC_INFO_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int atrackHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "ATrack Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_ATRACK_CMD, strlen(VLC_ATRACK_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int strackHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "VTrack Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_STRACK_CMD, strlen(VLC_STRACK_CMD));
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}

static int getTracksHandler(char** args, int nargs){
    int requestOk = 1;
    int vlc, omx /*, other*/;
    
    
    blog(LOG_INFO, "Prev Service requested");
    
    if(nargs != 2 || ((vlc   = strcasecmp(args[1], VLC_OPTION)        ) != 0  && 
                      (omx   = strcasecmp(args[1], OMXPLAYER_OPTION)  ) != 0) /* &&
                      (other = strcasecmp(nargs[2], OTHER_OPTION)     ) != 0) */ ){
        blog(LOG_ERROR, "Bad usage.");
        return -1;
    }
   
    if(vlc == 0){
        vlcProcCtx = updateProcessGlobalVar(vlcProcCtx, VLC_OPTION);
        
        /*
        if(isProcessRunning(vlcProcCtx) == 1)
            sendToProcess(vlcProcCtx, VLC_VOLDOWN_CMD, strlen(VLC_VOLUP_CMD));
        */
    }else if(omx == 0){
        updateProcessGlobalVar(omxProcCtx, OMXPLAYER_OPTION);
        
        /*
        if(isOmxRunning())
            sendToProcess(vlcProcCtx, OMX_PAUSE_CMD, sizeof(OMX_PAUSE_CMD));
        */
        
    } /* else if (other == 0){
       ...
    } */
    
    return requestOk;
}


static processContext* updateProcessGlobalVar(processContext *procCtx, char* procName){
    
    if(procCtx != NULL){
        blog(LOG_INFO, "%s appears be running. Checking status", procName);
        
        updateProcessStatus(procCtx);
        
        if(procCtx->status == RUNNING){
            blog(LOG_INFO, "Process %s (pid : %d) ready.", procName, procCtx->pid);
            return procCtx;
        }else{
            blog(LOG_INFO, "Process %s (pid : %d) anormal state", procName, procCtx->pid);
            
            if(procCtx->status == FINISHED)
                blog(LOG_INFO, "Process %s (pid : %d)  FINISHED successfully. Free resources", procName, procCtx->pid);
            else
                finishProcess(procCtx);
                
            freeProcessContext(procCtx);
        }
    }
    return NULL;
}
    
static int isProcessRunning(processContext *procCtx){
    if(procCtx == NULL){
        blog(LOG_INFO, "%s is not running", procCtx->binPath);
        return -1;
    }
    
    updateProcessStatus(procCtx);
    if (procCtx->status != RUNNING){
        blog(LOG_TRACE, "%s anormal status. updateVlcGlobalVar must have cleaned this process.", procCtx->status);
        return -1;
    }
    
    return 1;
}

static int selectMediaProcess4Uri(char *uri){
    return VLC;
}

static processContext* openVlc(char* uri){
    processContext* procCtx;
    
    blog(LOG_INFO, "Open Vlc. Uri : '%s'", uri);
    
    procCtx = createProcessContext(4, VLC_EXEC, "-I", "rc", uri);
    
    if(procCtx == NULL){
        blog(LOG_ERROR, "Error allocating for Vlc context.");
        return;
    }
    
    createProcess(procCtx);
    
    return procCtx;
}


static void quitVlc(processContext* procCtx){
    
    blog(LOG_INFO, "Closing Vlc");
    
    // Try to terminate process by command
    if(isProcessRunning(procCtx) == 1){
        sendToProcess(procCtx, VLC_QUIT_CMD, strlen(VLC_QUIT_CMD));
        
        procCtx = updateProcessGlobalVar(procCtx, VLC_OPTION);
    }
    
    // Force finish
    if(procCtx != NULL){
        blog(LOG_WARN, "Vlc cannot be closed. Force Finish");
        finishProcess(procCtx);
        freeProcessContext(procCtx);
    }
}
