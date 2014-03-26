

bbControlServer.o : bbControlServer.c
	$(CC) -c $(CCFLAGS) bbControlServer.c

bbMediaServer.o : bbMediaServer.c
	$(CC) -c $(CCFLAGS) bbMediaServer.c
	
bbLedServer.o : bbLedServer.c
	$(CC) -c $(CCFLAGS) bbLedServer.c



all : bbControlServer.o bbMediaServer.o bbLedServer.o
	$(CC) bbControlServer.o ../libs/stringutils.o ../libs/serversocket.o ../libs/logger.o ../libs/procmanager.o -o bbControlServer
	$(CC) bbMediaServer.o   ../libs/stringutils.o ../libs/serversocket.o ../libs/logger.o ../libs/procmanager.o -o bbMediaServer
	$(CC) bbLedServer.o     ../libs/logger.o                                                                    -o bbLedServer

clean:
	rm -rf *.o
	rm -rf bbControlServer bbMediaServer bbLedServer
	
install:
	cp bbControlServer /bb/bin
	cp bbMediaServer   /bb/bin
	cp bbLedServer     /bb/bin