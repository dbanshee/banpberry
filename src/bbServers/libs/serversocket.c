//#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include "serversocket.h"
#include "logger.h"


/*
 * Loop de ejecucion de escucha de conexiones.
 */
int socketServerLoop(serverSocketContext* ctx){
    
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    unsigned int clientLen;
    
    blog(LOG_INFO, "Initializing Server ... ");
    
    // Socket initialization
    if((ctx->listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        blog(LOG_ERROR, "Error opening socket");
        abortServer(ctx);
    }
    
    // Server address initialization
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(ctx->serverPort); 
    
    // Client address initialization
    clientLen = sizeof(client_addr);

    // Bind socket to address
    if(bind(ctx->listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
        blog(LOG_ERROR, "Error binding socket");
        abortServer(ctx);
    }

    // Listen queue
    if(listen(ctx->listenfd, 10) == -1){
        blog(LOG_ERROR, "Error listen on socket");
        abortServer(ctx);
    }
    
    blog(LOG_INFO, "Server initialized on port : %d... ", ctx->serverPort);
    
    // Conexion accept loop
    while(ctx->serverLoop){
        
        blog(LOG_INFO, "Waiting for connection ...");
        
        ctx->clientfd = accept(ctx->listenfd, (struct sockaddr *) &client_addr, &clientLen);
        
        /*
         * Se comprueba el estado del descriptor del socket. Si esta cerrado, se ignora el accept,
         * puesto que provendra de la terminacion del servidor. close() en finishServer() 
         */
        if(fcntl(ctx->listenfd, F_GETFL) != -1){
        
            if(ctx->clientfd == -1) 
                blog(LOG_ERROR, "Error on accept");

            blog(LOG_INFO, "Conexion received at port : %d", ntohs(client_addr.sin_port));

            // Request process
            ctx->requestHandler(ctx->clientfd);

            // Close client socket
            close(ctx->clientfd);
        }
    }
}

void finishServer(serverSocketContext* ctx){
    blog(LOG_INFO, "Finishing Server ...");
    ctx->serverLoop = 0;
    close(ctx->listenfd); // Close sobre el desc del socket fuerza la terminacion del bloqueo en el accept.
    close(ctx->clientfd);  // Close sobre la conexion del cliente activa
}

void abortServer(serverSocketContext* ctx){
    blog(LOG_ERROR, "Aborting Server ...");
    ctx->serverLoop = 0;
    close(ctx->listenfd); // Close sobre el desc del socket fuerza la terminacion del bloqueo en el accept.
    close(ctx->clientfd);  // Close sobre la conexion del cliente activa
    exit(-1);
}

ssize_t serverReadBuffer(int connfd, char* buff, size_t buffSize){
    
    if(fcntl(connfd, F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Input server socket is not ready.");
        return -1;
    }
    
    memset(buff, 0, buffSize);
    ssize_t nread = read(connfd, buff, buffSize);
    
    if(fcntl(connfd, F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Input server socket is not ready.");
        return -1;
    }else if(nread == -1){
        blog(LOG_WARN, "Empty Request.");
        buff[0] = '\0';
        return 0;
    }else{
        cleanLine(buff);
        blog(LOG_INFO, "Client Request : (bytes : %d) '%s'", nread, buff);
        return nread;
    }
}

ssize_t serverWriteBuffer(int connfd, char* buff, size_t buffSize){
    
    if(fcntl(connfd, F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Output server socket is not ready.");
        return -1;
    }   
    
    size_t nwrite = write(connfd, buff, buffSize);
    //fflush(connfd); ?
    
    if(nwrite == -1){
        blog(LOG_ERROR, "Error sending buffer.");
        return -1;
    }else{
        blog(LOG_INFO, "Server Response : (bytes : %d) '%s'", nwrite, buff);
        return nwrite;
    }
}

void cleanLine(char* line){
    int nread = strlen(line);
    
    if(nread > 0 && line[nread-1] == '\n')
        line[(nread--)-1] = '\0';
        
    if(nread > 0 && line[nread-1] == '\r')
        line[(nread--)-1] = '\0';
}