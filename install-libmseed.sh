#!/bin/sh
set -ex
VER=3.0.7
DIR=libmseed
wget --output-document=libmseed-${VER}.tar.gz \
     https://github.com/iris-edu/libmseed/archive/v${VER}.tar.gz
if [ ! -e $DIR ]; then
    mkdir $DIR
fi
tar -zxvf libmseed-${VER}.tar.gz -C $DIR --strip-components 1
cd libmseed && make static



