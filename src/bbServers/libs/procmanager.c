#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <dirent.h>
#include <readline/chardefs.h>
#include <stdarg.h>


#include "procmanager.h"
#include "logger.h"
#include "stringutils.h"

#define MAXBUFF      1024
#define MAXSOCMDSIZE 4096


static int closeNonStdDescriptors(pid_t pid, int fd1, int fd2);
static void sprintOSCommand(char *str, ssize_t buffSize, processContext *procCtx);


processContext* createProcessContext(int nargs, ...){
    int i = 0;
    processContext *procCtx = NULL;
    char* arg;

    
    if((procCtx = (processContext*) malloc(sizeof(processContext))) == NULL){
        blog(LOG_ERROR, "Error allocating memory for processContext");
        return NULL;
    }
    
    memset(procCtx, 0, sizeof(processContext));
    
    procCtx->binPath    = NULL;
    procCtx->args       = NULL;
    procCtx->status     = UNINITIALIZED;
    memset(procCtx->fd, 0, sizeof(char)*2);
    
    
    // Processing args
    if((procCtx->args =  (char**) malloc(sizeof(char*)*(nargs+1))) == NULL){
        blog(LOG_ERROR, "Error allocating memory for processContext args");
        free(procCtx);
        return NULL;
    }
        
    va_list vlist;
    va_start(vlist, nargs);

    for(i = 0; i < nargs; i++){
        arg = va_arg(vlist, char*);

        if(i == 0){ // binPath
            procCtx->binPath = (char *) malloc(sizeof(char*)*(strlen(arg)+1));
            strcpy(procCtx->binPath, arg);
        }

        procCtx->args[i] = (char *) malloc(sizeof(char*)*(strlen(arg)+1));
        strcpy(procCtx->args[i], arg);
    }
    
    va_end(vlist);
    procCtx->args[nargs] = NULL; // Really unnecesary. memset set all 0.
    return procCtx;
}

void freeProcessContext(processContext * procCtx){
    if(procCtx != NULL){
        free(procCtx->args); // Carefull
        free(procCtx);
    }

    procCtx == NULL;
}

static void sprintOSCommand(char *str, ssize_t buffSize, processContext *procCtx){
    int i;
    int offset = 0, acumOffset = 0;
    
    if(procCtx->args[0] != NULL){
        strcpy(str, procCtx->args[0]);
        acumOffset = strlen(str);
    }
    
    for(i = 1; procCtx->args[i] != NULL; i++){   
        offset = snprintf(str + acumOffset, buffSize - acumOffset, "%s %s", str + acumOffset, procCtx->args[i]);
        acumOffset += offset;
    }
}


void createProcess(processContext *procCtx){
    int p_stdin[2], p_stdout[2];
    pid_t pid;
    
    // Create 2 pipes
    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0){
        blog(LOG_ERROR, "Error creating pipes for subprocess");
        procCtx->status = ERROR;
        return;
    }
    
    // Create subprocess
    pid = fork();
    
    if(pid < 0){ // Error
        blog(LOG_ERROR, "Error creating subprocess");
        procCtx->status = ERROR;
        return;
    }else if(pid == 0){ // Child
       
        close(p_stdin[WRITE]);
        close(p_stdout[READ]);

        // Se cierran todos los descriptores del proceso excepto los (0,1,2) y los extremos necesarios de la pipe
        if(closeNonStdDescriptors(getpid(), p_stdin[READ], p_stdout[WRITE]) == -1){
            blog(LOG_ERROR, "Error Closing non std descriptors.");
            exit(-1);
        }
        
        // Se asocian las pipes a la entrada y salida estandar  y se redirige el error estandar a la salida estandar
        if(dup2(p_stdin[READ],   STDIN)  == -1 || 
           dup2(p_stdout[WRITE], STDOUT) == -1 ||
           dup2(p_stdout[WRITE], STDERR) == -1 ){
            
            blog(LOG_ERROR, "Error asociating std to pipes");
            exit(-1);
        }
        
        close(p_stdin[READ]);
        close(p_stdout[WRITE]);
        
        // Se cambia la imagen del proceso
        if(execvp(procCtx->binPath, procCtx->args) == -1)
            exit(-1);
        
    }else{ // Parent
        
        procCtx->status = RUNNING;
        procCtx->pid    = pid;
        procCtx->fd[0]  = p_stdin[WRITE];
        procCtx->fd[1]  = p_stdout[READ];
        
        close(p_stdin[READ]);
        close(p_stdout[WRITE]);
        
        
        char soCmd [MAXSOCMDSIZE];
        sprintOSCommand(soCmd, MAXSOCMDSIZE, procCtx);
        blog(LOG_INFO, "Executing OS Command : %s", soCmd);
        
    }    
}

void finishProcess(processContext* procCtx){
    
    blog(LOG_WARN, "Process %s (pid : %d) Sending SIGTERM signal", procCtx->binPath, procCtx->pid);
    signalProcess(procCtx, SIGTERM);

    updateProcessStatus(procCtx);
    if(procCtx->status != FINISHED){

        blog(LOG_WARN, "Process %s (pid : %d) status : %d after SIGTERM signal. Sending SIGKILL signal.", procCtx->binPath, procCtx->pid, procCtx->status);

        signalProcess(procCtx, SIGKILL);
        updateProcessStatus(procCtx);

        blog(LOG_WARN, "Process %s (pid : %d) status : %d after SIGKILL signal.", procCtx->binPath, procCtx->status);

        if(procCtx->status != FINISHED)
            blog(LOG_INFO, "Force free resources. Zombie risk?? Uuuhhh ...");
    }
}

static int closeNonStdDescriptors(pid_t pid, int fd1, int fd2){
    char procDirPath [256];
    DIR *dp;
    struct dirent *dirEntry;
    int efd;
    
    // Se recuperan los descriptores abiertos del proceso
    sprintf(procDirPath, "/proc/%d/fd", pid);
    
    if((dp = opendir(procDirPath)) == NULL){
        blog(LOG_ERROR, "Error opening directory : %s", procDirPath);
        return -1;
    }
    
    while((dirEntry = readdir(dp)) != NULL){
        efd = atoi(dirEntry->d_name);
        
        // Se cierran todos los descriptores no estandar, excepto los 2 pasados como argumento y el que se esta usando para navegar por los mismos
        if(efd > 2 && efd != fd1 && efd != fd2 && efd != dirfd(dp))
            close(efd);
    }
    
    closedir(dp);
    return 1;
}

void waitProcess(processContext *procCtx){
    pid_t   pid;
    int     status;
    char    buff [MAXBUFF];
    size_t  nread;
    
    // Se lee toda la salida estandar del proceso
    nread = readFromProcess(procCtx, buff, MAXBUFF-1);
    while(nread > 0){
        buff[nread] = '\0';
        blog(LOG_DEBUG, "<<<%s>>> : %s", procCtx->binPath, buff);
        nread = readFromProcess(procCtx, buff, MAXBUFF-1);
    }
    
    // Espera de señales del proceso
    blog(LOG_INFO, "Waiting end of process pid : %d", procCtx->pid);
    pid = waitpid(procCtx->pid, &status, 0);
    
    close(procCtx->fd[0]);
    close(procCtx->fd[1]);
    
    if(pid == 0){
        blog(LOG_INFO, "Blocking Wait for process pid %d return 0. Anormal case", procCtx->pid);
        procCtx->status = ERROR;
    }else if(pid == procCtx->pid){ // Status process has changed 
        
        if(WIFEXITED(status)){
            blog(LOG_INFO, "Process pid : %d finish normally", procCtx->pid);
            procCtx->status = FINISHED;
            
            if(WEXITSTATUS(status) != 0){
                blog(LOG_INFO, "Process pid : %d exit status : %d", procCtx->pid, WEXITSTATUS(status));
                procCtx->status = ERROR;
            }

            if(WIFSIGNALED(status))
                blog(LOG_INFO, "Process pid : %d signaled by signal %d", procCtx->pid, WTERMSIG(status));
        }else{
            blog(LOG_ERROR, "Process pid : %d exited anormally.", procCtx->pid);
            
            if(WIFSIGNALED(status)){
                blog(LOG_INFO, "Process pid : %d signaled by signal %d", procCtx->pid, WTERMSIG(status));
                procCtx->status = FINISHED;
            }else{
                blog(LOG_INFO, "Process pid : %d status anormall case.", procCtx->pid);
                procCtx->status = ERROR;
            }
        }
    }else{
        blog(LOG_ERROR, "Error waiting process pid : %d ", procCtx->pid);
        procCtx->status = ERROR;
    }
}

void updateProcessStatus(processContext *procCtx){
    pid_t pid;
    int status;
    
    if(procCtx->status == RUNNING){
        
        blog(LOG_INFO, "Waiting status of process pid : %d", procCtx->pid);
        pid = waitpid(procCtx->pid, &status, WNOHANG);

        if(pid == 0) // Status process has not changed
            return;
        
        if(pid == procCtx->pid){ // Status process has changed
            
            if(WIFEXITED(status)){
                blog(LOG_INFO, "Process pid : %d finish normally. Exit status : %d", procCtx->pid, WEXITSTATUS(status));
                procCtx->status = FINISHED;
                
                if(WIFSIGNALED(status))
                    blog(LOG_INFO, "Process pid : %d signaled by signal %d", procCtx->pid, WTERMSIG(status));
            }else{
                blog(LOG_ERROR, "Process pid : %d exited anormally", procCtx->pid);
                
                if(WIFSIGNALED(status)){
                    blog(LOG_INFO, "Process pid : %d signaled by signal %d", procCtx->pid, WTERMSIG(status));
                    procCtx->status = FINISHED;
                }else{
                    blog(LOG_INFO, "Process pid : %d status anormall case.", procCtx->pid);
                    procCtx->status = ERROR;
                }
            }
            
            close(procCtx->fd[0]);
            close(procCtx->fd[1]);
            
        }else{
            blog(LOG_ERROR, "Error waiting process pid : %d ", procCtx->pid);
            procCtx->status = ERROR;
            
            close(procCtx->fd[0]);
            close(procCtx->fd[1]);
        }
    }
}

ssize_t readFromProcess(processContext *procCtx, char *buff, size_t buffSize){
    
    if(procCtx->status == UNINITIALIZED){
        blog(LOG_ERROR, "Error. Read from UNINITIALIZED process.");
        return -1;
    }
    
    if(procCtx->status != RUNNING)
        blog(LOG_WARN, "Read from non RUNNING process.");
    
    if(fcntl(procCtx->fd[1], F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Read process pipe is not ready");
        return -1;
    }
    
    memset(buff, 0, buffSize);
    ssize_t nread = read(procCtx->fd[1], buff, buffSize);
    
    if(fcntl(procCtx->fd[1], F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Read Process pipe is not ready");
        return -1;
    }else if(nread == -1){
        blog(LOG_WARN, "Empty Read.");
        buff[0] = '\0';
        return 0;
    }else{
        blog(LOG_DEBUG, "Read From Process (%d bytes) : '%s'", nread, buff);
        return nread;
    }
}

ssize_t sendToProcess(processContext *procCtx, char *buff, size_t buffSize){
    
    if(procCtx->status == UNINITIALIZED){
        blog(LOG_ERROR, "Error. Send to UNINITIALIZED process.");
        return -1;
    }
    
    if(procCtx->status != RUNNING)
        blog(LOG_WARN, "Send from non RUNNING process.");
    
    if(fcntl(procCtx->fd[0], F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Write process pipe is not ready");
        return -1;
    }   
    
    if(buffSize == -1)
        buffSize = strlen(buff);
    
    size_t nwrite = write(procCtx->fd[0], buff, buffSize);
    //fsync(procCtx->fd[0]);
    
    if(nwrite == -1){
        blog(LOG_ERROR, "Error. Write to process pipe failed");
        return -1;
    }else{
        blog(LOG_DEBUG, "Write to process (%d bytes) : '%s'", nwrite, buff);
        return nwrite;
    }
}


void signalProcess(processContext *procCtx, int signal){
        
    if (procCtx->status == UNINITIALIZED){
        blog(LOG_WARN, "Kill (%d) to UNINITIALIZED process");
        return;
    } 
    
    if(procCtx->status == RUNNING)
        blog(LOG_WARN, "Kill (%d) to RUNNING process pid : %d",  signal, procCtx->pid);
    else if(procCtx->status == STOPPED)
        blog(LOG_WARN, "Kill (%d) to STOPPED process pid : %d",  signal, procCtx->pid);
    else if(procCtx->status == FINISHED)
        blog(LOG_WARN, "Kill (%d) to FINISHED process pid : %d", signal, procCtx->pid);
    else if(procCtx->status == ERROR)
        blog(LOG_WARN, "Kill (%d) to ERROR process pid : %d",    signal, procCtx->pid);
    else
        blog(LOG_ERROR, "Kill (%d) to UNKOWN process pid : %d",  signal, procCtx->pid);
       
    if(kill(procCtx->pid, signal))
        blog(LOG_INFO,  "Process pid : %d, signaled with : %d signal", signal);
    else
        blog(LOG_ERROR, "Failed signaling process pid : %d, with signal : %d", signal);
}


static int isNumeric (const char * s)
{
    if (s == NULL || *s == '\0' || isspace(*s))
        return 0;
    
    char * p;
    strtod (s, &p);
    return *p == '\0';
}

int existsProcess(const char *processName){
    char procDirPath[64], buff[MAXBUFF];
    DIR *dp;
    struct dirent *dirEntry;
    int exists = 0, fd;
    ssize_t nread;

    
    if((dp = opendir("/proc")) == NULL){
        blog(LOG_ERROR, "Error opening '/proc' directory");
        return -1;
    }
    
    while(exists == 0 && (dirEntry = readdir(dp)) != NULL){
        
        if(isNumeric(dirEntry->d_name)){    
            
            sprintf(procDirPath, "/proc/%s/comm", dirEntry->d_name);
            if((fd = open(procDirPath, O_RDONLY)) != 1){
                
                nread = read(fd, buff, MAXBUFF-1);
                if(nread == -1)
                    blog(LOG_ERROR, "Error reading '%s'", procDirPath);
                else if(nread == 0)
                    blog(LOG_WARN, "Empty content of '%s'", procDirPath);
                else{
                    buff[nread] = '\0';
                    cleanLine(buff);
                    
                    if(strcasecmp(processName, buff) == 0)
                        exists = 1;
                }
                
                close(fd);
            }
        }    
    }
    
    closedir(dp);
    return exists;
}

