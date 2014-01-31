#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <dirent.h>


#include "procmanager.h"
#include "logger.h"



void initializeProcessContex(processContext *procCtx){
    procCtx->binPath    = NULL;
    procCtx->args       = NULL;
    procCtx->status     = UNINITIALIZED;
    memset(procCtx->fd, 0, sizeof(char)*2);
}

void sprintSOCommand(char *str, ssize_t buffSize, processContext *procCtx){
    
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
    
    if(pid < 0){
        blog(LOG_ERROR, "Error creating subprocess");
        procCtx->status = ERROR;
        return;
    }else if(pid == 0){
        // Child
        
        close(p_stdin[WRITE]);
        close(p_stdout[READ]);
        
        // Se cierran todos los descriptores del proceso excepto los (0,1,2) y los extremos necesarios de la pipe
        if(closeNonStdDescriptors(getpid(), p_stdin[READ], p_stdout[WRITE]) == -1){
            blog(LOG_ERROR, "Error Closing non std descriptors.");
            procCtx->status = ERROR;
            exit(-1);
        }
        
        // Se asocian las pipes a la entrada y salida estandar  y se redirige el error estandar a la salida estandar
        if(dup2(p_stdin[READ],   STDIN)  == -1 || 
           dup2(p_stdout[WRITE], STDOUT) == -1 ||
           dup2(p_stdout[WRITE], STDERR) == -1 ){
            
            blog(LOG_ERROR, "Error asociating std to pipes");
            procCtx->status = ERROR;
            exit(-1);
        }
      
        close(p_stdin[READ]);
        close(p_stdout[WRITE]);

        // Print SO Command
        char soCmd [2048];
        
        
        // Se cambia la imagen del proceso
        procCtx->status = RUNNING;
        if(execvp(procCtx->binPath, procCtx->args) == -1){
            blog(LOG_ERROR, "Error changing image of subprocess");
            procCtx->status = ERROR;
            exit(-1);
        }
        
    }else{
        // Parent
        procCtx->pid    = pid;
        procCtx->fd[0]  = p_stdin[WRITE];
        procCtx->fd[1]  = p_stdout[READ];
        
        close(p_stdin[READ]);
        close(p_stdout[WRITE]);
    }    
}

static int closeNonStdDescriptors(pid_t pid, int fd1, int fd2){
    char procDirPath [256];
    DIR *dp;
    struct dirent *dirEntry;
    int efd;
    
    sprintf(procDirPath, "/proc/%d/fd", pid);
    
    if((dp = opendir(procDirPath)) == NULL){
        blog(LOG_ERROR, "Error opening %s directory.", pid);
        return -1;
    }
    
    while((dirEntry = readdir(dp)) != NULL){
        efd = atoi(dirEntry->d_name);
        
        // Se cierran todos los descriptores no estandar, excepto los 2 pasados como argumento y el que se esta usando para navegar por los mismos
        if(efd > 2 && efd != fd1 && efd != fd2 && efd != dirfd(dp))
            close(atoi(dirEntry->d_name));
    }
    
    closedir(dp);
    return 1;
}

void waitProcess(processContext *procCtx){
    pid_t pid;
    int status;
    
    blog(LOG_INFO, "Waiting end of process pid : %d", procCtx->pid);
    pid = waitpid(procCtx->pid, &status, 0);
    
    close(procCtx->fd[0]);
    close(procCtx->fd[1]);
    
    if(pid == 0){
        blog(LOG_INFO, "Blocking Wait for process pid %d return 0. Anormal case.", procCtx->pid);
        procCtx->status = ERROR;
    }else if(pid == procCtx->pid){ // Status process has changed 
        
        if(WIFEXITED(status)){
            blog(LOG_INFO, "Process pid : %d finish normally.", procCtx->pid);
            procCtx->status = FINISHED;

            if(WIFSIGNALED(status))
                blog(LOG_INFO, "Process pid : %d signaled by signal %d.", procCtx->pid, WTERMSIG(status));
        }else{
            blog(LOG_ERROR, "Process pid : %d exited anormally.", procCtx->pid);
            procCtx->status = ERROR;
        }
    }else{
        blog(LOG_ERROR, "Error waiting process pid : %d ", procCtx->pid);
        procCtx->status = ERROR;
    }
}

int getProcessStatus(processContext *procCtx){
    pid_t pid;
    int status;
    
    if(procCtx->status == RUNNING){
        
        blog(LOG_INFO, "Waiting end of process pid : %d", procCtx->pid);
        pid = waitpid(procCtx->pid, &status, WNOHANG);

        if(pid == 0) // Status process has not changed
            return procCtx->status;
        
        if(pid == procCtx->pid){ // Status process has changed
            
            if(WIFEXITED(status)){
                blog(LOG_INFO, "Process pid : %d finish normally.", procCtx->pid);
                procCtx->status = FINISHED;

                if(WIFSIGNALED(status))
                    blog(LOG_INFO, "Process pid : %d signaled by signal %d.", procCtx->pid, WTERMSIG(status));
            }else
                blog(LOG_ERROR, "Process pid : %d exited anormally.", procCtx->pid);
            
            close(procCtx->fd[0]);
            close(procCtx->fd[1]);
            
        }else{
            blog(LOG_ERROR, "Error waiting process pid : %d ", procCtx->pid);
            procCtx->status = ERROR;
            
            close(procCtx->fd[0]);
            close(procCtx->fd[1]);
        }
    }
    
    return procCtx->status;
}

ssize_t readFromProcess(processContext *procCtx, char *buff, size_t buffSize){
    
    if(fcntl(procCtx->fd[1], F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Read process pipe is not ready.");
        return -1;
    }
    
    memset(buff, 0, buffSize);
    ssize_t nread = read(procCtx->fd[1], buff, buffSize);
    
    if(fcntl(procCtx->fd[1], F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Read Process pipe is not ready.");
        return -1;
    }else if(nread == -1){
        blog(LOG_WARN, "Empty Read.");
        buff[0] = '\0';
        return 0;
    }else{
        cleanLine(buff);
        blog(LOG_DEBUG, "Read From Process: (bytes : %d) '%s'", nread, buff);
        return nread;
    }
}

ssize_t sendToProcess(processContext *procCtx, char *buff, size_t buffSize){
    
    if(fcntl(procCtx->fd[0], F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Write process pipe is not ready.");
        return -1;
    }   
    
    if(buffSize == -1)
        buffSize = strlen(buff);
    
    size_t nwrite = write(procCtx->fd[0], buff, buffSize);
    
    if(nwrite == -1){
        blog(LOG_ERROR, "Error. Write on process pipe failed.");
        return -1;
    }else{
        blog(LOG_DEBUG, "Write to process : (bytes : %d) '%s'", nwrite, buff);
        return nwrite;
    }
}

