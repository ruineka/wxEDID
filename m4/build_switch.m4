dnl serial 4
dnl
dnl author: Tomasz Pawlak
dnl e-mail: tomasz.pawlak@wp.eu
dnl 
dnl AC_BUILD_SWITCH v2.2 (2020.11.21)
dnl extension for autoconf:
dnl Implementation of Debug/Release build model.
dnl
dnl AC_BUILD_SWITCH(
dnl <debug flags:>   [CXXFLAGS], [CFLAGS], [CPPFLAGS], [LDFLAGS],
dnl <release flags:> [CXXFLAGS], [CFLAGS], [CPPFLAGS], [LDFLAGS]
dnl                )
AC_DEFUN_ONCE([AC_BUILD_SWITCH],
[
  AC_ARG_ENABLE([build-mode],
                AS_HELP_STRING([--enable-build-mode=@<:@release/debug/none@:>@], [
                  Use predefined flags for release or debug build, none=no predefined flags.
                  [default=release]
                ]),
                [enable_flags=$enableval],
                [enable_flags=release])
  AS_MESSAGE([(i) Use predefined build flags? ... $enable_flags])
  AS_IF([test "x$enable_flags" = "xdebug"],
  [
    AS_VAR_SET([ac_bsw_CXXFLAGS], [" $1 "])
    AS_VAR_SET([ac_bsw_CFLAGS],   [" $2 "])
    AS_VAR_SET([ac_bsw_CPPFLAGS], [" $3 "])
    AS_VAR_SET([ac_bsw_LDFLAGS],  [" $4 "])
    AS_MESSAGE([(i) Debug CXXFLAGS   = ($ac_bsw_CXXFLAGS)])
    AS_MESSAGE([(i) Debug CFLAGS     = ($ac_bsw_CFLAGS)  ])
    AS_MESSAGE([(i) Debug CPPFLAGS   = ($ac_bsw_CPPFLAGS)])
    AS_MESSAGE([(i) Debug LDFLAGS    = ($ac_bsw_LDFLAGS) ])
  ])
  AS_IF([test "x$enable_flags" = "xrelease"],
  [
    AS_VAR_SET([ac_bsw_CXXFLAGS], [" $5 "])
    AS_VAR_SET([ac_bsw_CFLAGS],   [" $6 "])
    AS_VAR_SET([ac_bsw_CPPFLAGS], [" $7 "])
    AS_VAR_SET([ac_bsw_LDFLAGS],  [" $8 "])
    AS_MESSAGE([(i) Release CXXLAGS  = ($ac_bsw_CXXFLAGS)])
    AS_MESSAGE([(i) Release CFLAGS   = ($ac_bsw_CFLAGS)  ])
    AS_MESSAGE([(i) Release CPPFLAGS = ($ac_bsw_CPPFLAGS)])
    AS_MESSAGE([(i) Release LDFLAGS  = ($ac_bsw_LDFLAGS) ])
  ])
  AS_IF([test "x$enable_flags" = "xnone"],
  [
    AS_VAR_SET([ac_bsw_CXXFLAGS], [" "])
    AS_VAR_SET([ac_bsw_CFLAGS],   [" "])
    AS_VAR_SET([ac_bsw_CPPFLAGS], [" "])
    AS_VAR_SET([ac_bsw_LDFLAGS],  [" "])
  ])
  AS_VAR_APPEND([CXXFLAGS], ["$ac_bsw_CXXFLAGS"])
  AS_VAR_APPEND([CFLAGS],   ["$ac_bsw_CFLAGS"  ])
  AS_VAR_APPEND([CPPFLAGS], ["$ac_bsw_CPPFLAGS"])
  AS_VAR_APPEND([LDFLAGS],  ["$ac_bsw_LDFLAGS" ])
  AS_MESSAGE([(i) Final CXXFLAGS   = ($CXXFLAGS)])
  AS_MESSAGE([(i) Final CFLAGS     = ($CFLAGS)  ])
  AS_MESSAGE([(i) Final CPPFLAGS   = ($CPPFLAGS)])
  AS_MESSAGE([(i) Final LDFLAGS    = ($LDFLAGS) ])
])dnl AC_BUILD_SWITCH

