#!/bin/bash

mkdir -p build
cd build
../configure --prefix=${PWD}/release/ && make

if [ "$1" = "install" ];
then
  make install
fi