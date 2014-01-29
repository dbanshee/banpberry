

bbControlServer.o : bbControlServer.c
	gcc -c bbControlServer.c

bbMediaServer.o : bbMediaServer.c
	gcc -c bbMediaServer.c


all : bbControlServer.o #bbMediaServer.o
	 gcc bbControlServer.o ../libs/stringutils.o ../libs/serversocket.o ../libs/logger.o ../libs/procmanager.o -o bbControlServer
#	 gcc bbMediaServer.o ../libs/serversocket.o -o bbMediaServer

clean:
	rm -rf *.o
	rm -rf bbControlServer bbMediaServer
