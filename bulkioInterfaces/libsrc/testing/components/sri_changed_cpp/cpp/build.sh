#!/bin/sh

# Create the Makefile if necessary
if [ ! -e Makefile ]; then
  ./reconf
  ./configure
fi

make -j $*

