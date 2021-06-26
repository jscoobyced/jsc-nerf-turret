lib_LTLIBRARIES = libjscutils.la
libjscutils_la_SOURCES = logger.c
include_HEADERS = logger.h
libjscutils_la_CPPFLAGS = ${GLIB_CFLAGS}
libjscutils_la_CPPFLAGS += -pthread -I/usr/include/libmount -I/usr/include/blkid
libjscutils_la_CPPFLAGS += ${GIOUNIX_CFLAGS}
libjscutils_la_LIBADD = ${GLIB_LIBS}