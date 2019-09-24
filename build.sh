#!/bin/sh

./configure
RETVAL=$?
if [ $RETVAL -ne 0 ]; then
    cat config.log
    exit $RETVAL
fi

make
RETVAL=$?
if [ $RETVAL -ne 0 ]; then
    exit $RETVAL
fi

make test
RETVAL=$?
if [ $RETVAL -ne 0 ]; then
    cat ./test-suite.log
    exit $RETVAL
fi


