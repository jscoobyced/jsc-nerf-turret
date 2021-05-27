#!/bin/bash

rm -Rf autom4te.cache m4 config aclocal.m4 compile config.* \
      configure depcomp install-sh ltmain.sh Makefile.in missing \
      stamp-h1 Makefile libtool build \
      jscturret/Makefile.in jscturret/.deps jscturret/Makefile \
      btservice/Makefile.in btservice/Makefile btservice/.deps \
      btservice/.libs btservice/libbtservice_la* btservice/libbtservice.la \
      stepper/Makefile.in stepper/Makefile stepper/.deps \
      stepper/.libs stepper/libstepper_la* stepper/libstepper.la \
      utils/Makefile.in utils/Makefile utils/.deps \
      utils/.libs utils/libutils_la* utils/libutils.la
      