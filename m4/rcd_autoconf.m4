dnl serial 4
dnl
dnl extension for autoconf: integration with rcd_autogen
dnl
dnl rcd_autoconf.m4
dnl version: 1.3 (2020.11.21)
dnl
dnl author: Tomasz Pawlak
dnl e-mail: tomasz.pawlak@wp.eu
dnl
dnl Usage:
dnl AC_OPT_RCD_AUTOGEN([ path/to/rcd_autogen ])
dnl
AC_DEFUN_ONCE([AC_OPT_RCD_AUTOGEN],[
  AC_ARG_ENABLE([rcdgen-mode],
              AS_HELP_STRING([--enable-rcdgen-mode=@<:@full/basic/dummy/skip/clean@:>@], [
                full: generate struct of rcode messages and releated accessor functions code;
                basic: struct of rcode messages only (+ tiny accessors);
                dummy: mainly for libs: dummy fuctions only, to keep the interface for linking;
                skip: do not run the rcd_autogen;
                clean: remove all rcd_autogen output files.
                [default=full]
              ]),
              [rcd_mode=$enableval],
              [rcd_mode=full])

  AC_CHECK_PROG(ac_have_bash, bash, yes, no)
  AS_IF([test "$ac_have_bash" = "yes"], [],
        [AC_MSG_ERROR((E) Bash is required to run rcd_autogen, 1)])
  AS_UNSET(ac_have_bash)

  AS_MESSAGE([(i) rcd_autogen mode:   $rcd_mode])
  AS_MESSAGE([(i) rcd_autogen CPP   ='$CPP'    ])
  AS_MESSAGE([(i) rcd_autogen CXXCPP='$CXXCPP' ])
  
  AS_VAR_SET([ac_rcd_autogen_path], ["$1"])

dnl NOTE: It's possible to export RCDGEN_PP_ARGS=<any args>
dnl       before invoking configure script
  AS_VAR_APPEND([RCDGEN_PP_ARGS], [[" $CPPFLAGS"]])

  AS_VAR_APPEND([CPPFLAGS], [[" -D_USE_RCD_AUTOGEN"]])dnl see rcode.h

dnl NOTE: preprocessor command defined by configure overrides
dnl       the --preproc|-pp setting in rcd_autogen, but
dnl       the --pp-args|-pa are always appended to PP arg list.
  AS_IF([test "pp$CC" != "pp"],
  [ export RCDGEN_CPP=$CPP ],[])

  AS_IF([test "pp$CXXCPP" != "pp"],
  [ export RCDGEN_CXXCPP=$CXXCPP ],[])

dnl NOTE: Modes: full/basic/dummy can be overridden by rcd_autogen
dnl       option: --run-mode|-md, if it's set.
dnl       This allows to have per-target modes defined (in multi-target projects)
dnl       Modes: skip/clean are never overridden.
dnl       There's no 'RCDGEN_FULL', because 'full' is the default mode.
  AS_IF([test "$rcd_enable" = "basic"],
  [ export RCDGEN_BASIC=1 ],[])

  AS_IF([test "$rcd_enable" = "dummy"],
  [ export RCDGEN_DUMMY=1 ],[])

  AS_IF([test "$rcd_enable" = "skip"],
  [ AS_UNSET(ac_rcd_autogen_path) ],[])

  AS_IF([test "$rcd_enable" = "clean"],
  [ export RCDGEN_CLEAN=1 ],[])
])dnl AC_OPT_RCD_AUTOGEN

dnl run the rcd_autogen against given target
dnl
dnl Usage:
dnl 1. Pass the arguments directly to rcd_autogen:
dnl    AC_RCODE_AUTOGEN_TARGET([ -l=C -pa='-Wfatal-errors' -rd=. -st=/src ])
dnl 2. Use a file with arguments for rcd_autogen: (.rcdgen_cfg extension is mandatory)
dnl    AC_RCODE_AUTOGEN_TARGET([ src/target_name.rcdgen_cfg ])
AC_DEFUN([AC_RCD_AUTOGEN_TARGET],[
  AS_IF([test "x$ac_rcd_autogen_path" = "x"], dnl mode=skip
  [],[
    AS_IF([bash -c "$ac_rcd_autogen_path $1"], [],
          [AC_MSG_ERROR((E) rcd_autogen failed, 1)])
  ])
])dnl AC_RCD_AUTOGEN_TARGET

