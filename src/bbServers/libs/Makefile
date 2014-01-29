stringutils.o : stringutils.c
	gcc -c stringutils.c

serversocket.o : serversocket.c
	gcc -c serversocket.c
	
logger.o : logger.c
	gcc -c logger.c
	
procmanager.o : procmanager.c
	gcc -c procmanager.c

all: stringutils.o serversocket.o logger.o procmanager.o

clean :
	rm -rf *.o
