export CC = gcc
export CCFLAGS =

debug: CCFLAGS=-g
debug: all

all : 
	cd libs; $(MAKE) all
	cd servers; $(MAKE) all

clean:
	cd libs; $(MAKE) clean
	cd servers; $(MAKE) clean

install:
	cd servers; $(MAKE) install
	
