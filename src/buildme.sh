#!/bin/bash

mkdir -p build
cd build
../configure --prefix=${PWD}/build/ && make

if [ "$1" = "install" ];
then
  make install
fi