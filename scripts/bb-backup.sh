#! /bin/bash

DEST=banshee@192.168.15.6

rm -rf /bb-src.tar.gz || exit -1
tar czvf /bb-src.tar.gz /bb-src || exit -1
ssh $DEST "mv /media/E1/src/banpberry/bb-src.tar.gz /media/E1/src/banpberry/bb-src.tar.gz.back" || exit -1
scp /bb-src.tar.gz $DEST:/media/E1/src/banpberry/ || exit -1
