################################################
#
# Banpberry Servers Makefile
#   Banshee - 2014
#
# 
# bbControlServer
#   General server to start/stop/restart services
#
# bbMediaServer
#   Media servers to control vlc and omxplayer
#

export CC=gcc
export CCFLAGS=-g

debug: all
	
release: CCFLAGS=-O3
release: all

all: 
	cd libs; $(MAKE) all
	cd servers; $(MAKE) all

clean:
	cd libs; $(MAKE) clean
	cd servers; $(MAKE) clean

install:
	cd servers; $(MAKE) install
	
