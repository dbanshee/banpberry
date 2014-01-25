#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>


#include "procmanager.h"
#include "logger.h"



void initializeProcessContex(processContext* procCtx){
    procCtx->binPath    = NULL;
    procCtx->args       = NULL;
    procCtx->status     = UNINITIALIZED;
    memset(procCtx->fd, 0, sizeof(char)*2);
}


void createProcess(processContext* procCtx){

    int p_stdin[2], p_stdout[2];
    pid_t pid;
    
    // Create 2 pipes
    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0){
        blog(LOG_ERROR, "Error creating pipes for subprocess");
        return;
    }
    
    // Create subprocess
    pid = fork();
    
    if(pid < 0){
        blog(LOG_ERROR, "Error creating subprocess");
        return;
    }else if(pid == 0){
        // Child
        
        close(p_stdin[WRITE]);
        close(p_stdout[READ]);
        
        // Se asocian las pipes a la entrada y salida estandar  y se redirige el error estandar a la salida estandar
        if(dup2(p_stdin[READ],   STDIN)  == -1 || 
           dup2(p_stdout[WRITE], STDOUT) == -1 ||
           dup2(p_stdout[WRITE], STDERR) == -1 ){
            
            blog(LOG_ERROR, "Error asociating std to pipes");
            return;
        }
      
        close(p_stdin[READ]);
        close(p_stdout[WRITE]);

        
        // Se cambia la imagen del proceso
        if(execvp(procCtx->binPath, procCtx->args) == -1){
            blog(LOG_ERROR, "Error changing image of subprocess");
            return;
        }
        
    }else{
        // Parent
        procCtx->pid    = pid;
        procCtx->status = RUNNING;
        procCtx->fd[0]  = p_stdin[WRITE];
        procCtx->fd[1]  = p_stdout[READ];
        
        close(p_stdin[READ]);
        close(p_stdout[WRITE]);
    }    
}


void waitProcess(processContext* procCtx){
    pid_t pid;
    int status;
    
    blog(LOG_INFO, "Waiting end of process pid : %d", procCtx->pid);
    pid = waitpid(procCtx->pid, &status, 0);
    
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

int getProcessStatus(processContext* procCtx){
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
            
        }else{
            blog(LOG_ERROR, "Error waiting process pid : %d ", procCtx->pid);
            procCtx->status = ERROR;
        }
    }
    
    return procCtx->status;
}

ssize_t readFromProcess(processContext* procCtx, char* buff, size_t buffSize){
    
    if(fcntl(procCtx->fd[0], F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Read process pipe is not ready.");
        return -1;
    }
    
    memset(buff, 0, buffSize);
    ssize_t nread = read(procCtx->fd[0], buff, buffSize);
    
    if(fcntl(procCtx->fd[0], F_GETFL) == -1){
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

ssize_t sendToProcess(processContext* procCtx, char* buff, size_t buffSize){
    
    if(fcntl(procCtx->fd[0], F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Write process pipe is not ready.");
        return -1;
    }   
    
    size_t nwrite = write(procCtx->fd[0], buff, buffSize);
    
    if(nwrite == -1){
        blog(LOG_ERROR, "Error. Write on process pipe failed.");
        return -1;
    }else{
        blog(LOG_DEBUG, "Write to process : (bytes : %d) '%s'", nwrite, buff);
        return nwrite;
    }
}
