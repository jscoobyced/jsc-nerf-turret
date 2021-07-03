#!/bin/bash

rm -Rf autom4te.cache m4 config aclocal.m4 compile config.* \
      configure depcomp install-sh ltmain.sh Makefile.in missing \
      stamp-h1 Makefile libtool build \
      jscturret/Makefile.in jscturret/.deps jscturret/Makefile \
      jsc-bluetooth/src/btservice/Makefile.in \
      jsc-bluetooth/src/btservice/Makefile \
      jsc-bluetooth/src/btservice/.deps \
      jsc-bluetooth/src/btservice/.libs \
      jsc-bluetooth/src/btservice/libbtservice_la* \
      jsc-bluetooth/src/btservice/libbtservice.la \
      stepper/Makefile.in stepper/Makefile stepper/.deps \
      stepper/.libs stepper/libstepper_la* stepper/libstepper.la \
      jsc-bluetooth/src/utils/Makefile.in \
      jsc-bluetooth/src/utils/Makefile \
      jsc-bluetooth/src/utils/.deps \
      jsc-bluetooth/src/utils/.libs utils/libutils_la* \
      jsc-bluetooth/src/utils/libutils.la
      