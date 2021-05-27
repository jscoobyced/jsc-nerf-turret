#!/bin/sh

./cleanup.sh
mkdir -p config m4
autoreconf --install -I config -I m4