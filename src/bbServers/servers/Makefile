

bbControlServer.o : bbControlServer.c
	$(CC) -c $(CCFLAGS) bbControlServer.c

bbMediaServer.o : bbMediaServer.c
	$(CC) -c $(CCFLAGS) bbMediaServer.c


all : bbControlServer.o bbMediaServer.o
	$(CC) bbControlServer.o ../libs/stringutils.o ../libs/serversocket.o ../libs/logger.o ../libs/procmanager.o -o bbControlServer
	$(CC) bbMediaServer.o   ../libs/stringutils.o ../libs/serversocket.o ../libs/logger.o ../libs/procmanager.o -o bbMediaServer

clean:
	rm -rf *.o
	rm -rf bbControlServer bbMediaServer
	
install:
	cp bbControlServer /bb/bin
	cp bbMediaServer   /bb/bin