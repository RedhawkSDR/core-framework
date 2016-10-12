#!/bin/bash

# Create the Makefile if necessary
if [ ! -e Makefile ]; then
  ./reconf
  ./configure
fi

if [ $# == 1 ]; then
    if [ $1 == 'clean' ]; then
        make distclean
    else
        make -j $*
    fi
else
    make -j $*
fi
