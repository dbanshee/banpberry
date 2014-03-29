

bbControlServer.o : bbControlServer.c
	$(CC) -c $(CCFLAGS) bbControlServer.c

bbMediaServer.o : bbMediaServer.c
	$(CC) -c $(CCFLAGS) bbMediaServer.c
	
bbLedServer.o : bbLedServer.c
	$(CC) -c $(CCFLAGS) -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux bbLedServer.c



all : bbControlServer.o bbMediaServer.o bbLedServer.o
	$(CC) bbControlServer.o ../libs/stringutils.o ../libs/serversocket.o ../libs/logger.o ../libs/procmanager.o -o bbControlServer
	$(CC) bbMediaServer.o   ../libs/stringutils.o ../libs/serversocket.o ../libs/logger.o ../libs/procmanager.o -o bbMediaServer
	$(CC) -L/opt/vc/lib -lbcm_host bbLedServer.o  -lm                    ../libs/logger.o                       -o bbLedServer

clean:
	rm -rf *.o
	rm -rf bbControlServer bbMediaServer bbLedServer
	
install:
	cp bbControlServer /bb/bin
	cp bbMediaServer   /bb/bin
	cp bbLedServer     /bb/bin
