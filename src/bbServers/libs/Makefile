stringutils.o : stringutils.c
	$(CC) -c $(CCFLAGS) stringutils.c

serversocket.o : serversocket.c
	$(CC) -c $(CCFLAGS) serversocket.c
	
logger.o : logger.c
	$(CC) -c $(CCFLAGS) logger.c
	
procmanager.o : procmanager.c
	$(CC) -c $(CCFLAGS) procmanager.c

all: stringutils.o serversocket.o logger.o procmanager.o

clean :
	rm -rf *.o
