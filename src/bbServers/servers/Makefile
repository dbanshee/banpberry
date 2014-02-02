

bbControlServer.o : bbControlServer.c
	$(CC) -c $(CCFLAGS) bbControlServer.c

bbMediaServer.o : bbMediaServer.c
	$(CC) -c $(CCFLAGS) bbMediaServer.c


all : bbControlServer.o #bbMediaServer.o
	 $(CC) bbControlServer.o ../libs/stringutils.o ../libs/serversocket.o ../libs/logger.o ../libs/procmanager.o -o bbControlServer
#	 gcc bbMediaServer.o ../libs/serversocket.o -o bbMediaServer

clean:
	rm -rf *.o
	rm -rf bbControlServer bbMediaServer
	
install:
	cp bbControlServer /bb/bin