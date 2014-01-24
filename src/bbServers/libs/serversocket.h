/* 
 * File:   serversocket.h
 * Author: banshee
 *
 * Created on January 21, 2014, 10:57 PM
 */

#ifndef SERVERSOCKET_H
#define	SERVERSOCKET_H

#define REQUEST_OK      "REQUEST-OK"
#define REQUEST_ERROR   "REQUEST-ERROR"


typedef struct serverSocketContext {
    int serverPort;
    int (*requestHandler)(int);
    int serverLoop;
    int listenfd;
    int clientfd; // Cambiar por array o lista cuando se permitan conexiones concurrentes.
} serverSocketContext;


int  socketServerLoop(serverSocketContext* ctx);
void abortServer(serverSocketContext* ctx);
void finishServer(serverSocketContext* ctx);

ssize_t serverReadBuffer(int connfd, char* buff, size_t buffSize);
ssize_t serverWriteBuffer(int connfd, char* buff, size_t buffSize);
void cleanLine(char* line);


#endif	/* SERVERSOCKET_H */
