#!/bin/sh

MSEED_V="3.0.7"
SACIO_V="1.0.2"
MSEED="libmseed-v${MSEED_V}"
SACIO="sacio-v${SACIO_V}"

if [ ! -e libmseed ]; then
    echo "Downloading ${MSEED}.tar.gz"
    wget -q -O ${MSEED}.tar.gz https://github.com/iris-edu/libmseed/archive/v${MSEED_V}.tar.gz
    tar zxf ${MSEED}.tar.gz
    mv libmseed-${MSEED_V} libmseed
fi
if [ ! -e sacio ]; then
    echo "Downloading ${SACIO}.tar.gz"
    wget -q -O ${SACIO}.tar.gz https://github.com/savage13/sacio/archive/v${SACIO_V}.tar.gz
    tar zxf ${SACIO}.tar.gz

    mv sacio-${SACIO_V} sacio
fi

if [ ! -e libmseed/libmseed.a ]; then
    echo "Compiling libmseed"
    cd libmseed
    make -s
    cd ..
fi

if [ ! -e sacio/libsacio_bsd.a ]; then
    echo "Compiling sacio"
    cd sacio
    ./configure && make -s && make -s test
    cd ..
fi

./configure LDFLAGS="-Llibmseed -Lsacio" CFLAGS="-Isacio" && make && make test
