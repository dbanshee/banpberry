/* 
 * File:   procmanager.h
 * Author: banshee
 *
 * Created on January 24, 2014, 8:02 PM
 */

#ifndef PROCMANAGER_H
#define	PROCMANAGER_H

#include <stdio.h>

#define UNINITIALIZED  0
#define RUNNING        1
#define STOPPED        2
#define FINISHED       3
#define ERROR          4

#define READ  0
#define WRITE 1

#define STDIN  0
#define STDOUT 1
#define STDERR 2


typedef struct processContext {
    pid_t  pid;
    char   *binPath;
    char   **args;
    int    status;
    int    fd [2];
} processContext;

// Public Functions
void initializeProcessContex(processContext *procCtx);
void createProcess(processContext *procCtx);
void waitProcess(processContext *procCtx);
int existsProcess(const char* processName);

int sendToProcess(processContext   *procCtx, char *buff, size_t buffSize);
int readFromProcess(processContext *procCtx, char *buff, size_t buffSize);

#endif	/* PROCMANAGER_H */

