AC_INIT([JSC Nerf Turret], 1.0)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([foreign])
LT_INIT
LT_PREREQ([2.2])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
                 utils/Makefile
                 stepper/Makefile
                 jscturret/Makefile
                 btservice/Makefile])
AC_OUTPUT
AC_CONFIG_MACRO_DIRS([m4])
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.0)
PKG_CHECK_MODULES(DBUS, dbus-1 >= 1.0)
PKG_CHECK_MODULES(DBUSGLIB, dbus-glib-1 >= 0.1)
PKG_CHECK_MODULES(GIO, gio-2.0 >= 2.0)
PKG_CHECK_MODULES(GIOUNIX, gio-unix-2.0 >= 2.0)
AC_OUTPUT