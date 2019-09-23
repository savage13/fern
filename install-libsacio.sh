#!/bin/sh
set -ex
DIR=sacio
wget --output-document=libsacio-master.tar.gz \
     https://github.com/savage13/sacio/tarball/master
if [ ! -e $DIR ]; then
    mkdir $DIR
fi
tar -zxvf libsacio-master.tar.gz -C $DIR --strip-components 1
cd sacio && ./configure && make




