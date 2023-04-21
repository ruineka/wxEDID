dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

dnl wxEDID - Extended Display Identification Data editor

AC_PREREQ([2.65])
AC_INIT([wxEDID], [0.0.27], [tomasz.pawlak@wp.eu])
AC_COPYRIGHT([Copyright (C) 2014-2022 Tomasz Pawlak, e-mail: tomasz.pawlak@wp.eu])
AC_CONFIG_SRCDIR([src/wxEDID_Main.cpp])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([subdir-objects no-dependencies foreign nostdinc silent-rules])

dnl disable the default "-g -O2" flags. (after AC_INIT, before AC_PROG_CXX)
: ${CXXFLAGS=""}
: ${CPPFLAGS=""}
AC_PROG_CXX
AC_LANG([C++])
AS_VAR_SET([ac_lang_cflags], [$CFLAGS])

AC_OPT_RCD_AUTOGEN([src/rcode/rcd_autogen])

dnl AC_BUILD_SWITCH(
dnl <debug flags:>   [CXXFLAGS], [CFLAGS], [CPPFLAGS], [LDFLAGS],
dnl <release flags:> [CXXFLAGS], [CFLAGS], [CPPFLAGS], [LDFLAGS]
dnl                )
AC_BUILD_SWITCH(
  [ -fno-exceptions -g ], [], [ -Wall -DDBG_LEVEL_1 -DDBG_LEVEL_2], [],
  [ -fno-exceptions -O1], [], [ -Wall -Wfatal-errors], [ -Wl,-s])

AC_SUBST([ac_lang_cflags])

AS_MESSAGE([(i) Final CFLAGS = ($ac_lang_cflags) ])
AS_MESSAGE([(i) Final LDFLAGS = ($LDFLAGS) ])

dnl Checks for libraries.

WX_CONFIG_CHECK([3.1.0], [wxWin=1], [wxWin=0], [std,aui])
if test "$wxWin" != 1; then
   
   WX_CONFIG_CHECK([3.0.0], [wxWin=1], [wxWin=0], [std,aui])
   if test "$wxWin" != 1; then
      AC_MSG_ERROR([
         wxWidgets must be installed on your system
         but wx-config script couldn't be found.
         Please check that wx-config is in path, the directory
         where wxWidgets libraries are installed (returned by
         'wx-config --libs' command) is in LD_LIBRARY_PATH or
         equivalent variable and wxWidgets version is 3.0.0 or above.
      ])
   fi
fi

CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"

LIBS="$LIBS $WX_LIBS"

AS_VAR_APPEND([RCDGEN_PP_ARGS], [" $WX_CPPFLAGS"])
export RCDGEN_PP_ARGS
AC_RCD_AUTOGEN_TARGET([src/wxedid.rcdgen_cfg])

AC_CONFIG_FILES([
  Makefile
])
AC_OUTPUT

