#include "stringutils.h"

#include <stdio.h>
#include <stdlib.h>


void cleanLine(char *line){
    int nread = strlen(line);
    
    if(nread > 0 && line[nread-1] == '\n')
        line[(nread--)-1] = '\0';
        
    if(nread > 0 && line[nread-1] == '\r')
        line[(nread--)-1] = '\0';   
}

// Not Thread safe!!
// Need to free
char **splitLine(char *line, char *delim){
    int maxSize = 16;
    
    char **splitedLine = (char**) malloc(sizeof(char*)*maxSize);
    memset(splitedLine, 0, sizeof(char*)*maxSize);
    
    int cnt     = 0;
    char* tok = strtok(line, delim);
    while(tok != NULL){
        
        if(cnt >= maxSize)
            splitedLine = realloc(splitedLine, sizeof(char*)*(cnt+maxSize));
        
        splitedLine[cnt++] = tok;
        tok = strtok(NULL, delim);
    }
    
    return splitedLine;
}