#!/bin/bash

mkdir build
cd build
../configure --prefix=${PWD}/build/ && make