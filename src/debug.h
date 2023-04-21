/* Declarations for debug.h

   Copyright (C) 2014-2022 Tomasz Pawlak, e-mail: tomasz.pawlak@wp.eu
   License: GNU General Public License version 3 (GPLv3+)

   debug.h
   v0.2 (2014-09-12)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this file; If not, see <http://www.gnu.org/licenses/>.
   */

#ifndef DEBUG_H
#define DEBUG_H 1

#include <stdio.h>

#if __WORDSIZE == 64
   #define PR_SIZET  "lu"
   #define PR_SSIZET	"ld"
   #define PR_I64    "lu"
   #define PR_SI64   "ld"
#else
   #define PR_SIZET	"u"
   #define PR_SSIZET "d"
   #define PR_I64    "llu"
   #define PR_SI64   "lld"
#endif

#define __debug_log_fd stdout
#define __debug_err_fd stderr

#ifndef __debug_dbg0_fd
#define __debug_dbg0_fd stderr
#endif
#ifndef __debug_dbg1_fd
#define __debug_dbg1_fd stderr
#endif
#ifndef __debug_dbg2_fd
#define __debug_dbg2_fd stderr
#endif

#ifdef DBG_LEVEL_ALL
   #pragma message "DBG_LEVEL_ALL on"
   #ifndef DBG_LEVEL_0
      #define DBG_LEVEL_0
   #endif
   #ifndef DBG_LEVEL_1
      #define DBG_LEVEL_1
   #endif
   #ifndef DBG_LEVEL_2
      #define DBG_LEVEL_2
   #endif
#endif

#ifdef DBG_LEVEL_0
   #pragma message "DBG_LEVEL_0 on"
   #ifndef DBG_MODE
      #define DBG_MODE
   #endif
   #define DBG0_PRINT(...) \
      fprintf(__debug_dbg0_fd, __VA_ARGS__)
#else
   #define DBG0_PRINT(...)
#endif

#ifdef DBG_LEVEL_1
   #pragma message "DBG_LEVEL_1 on"
   #ifndef DBG_MODE
      #define DBG_MODE
   #endif
   #define DBG1_PRINT(...) \
      fprintf(__debug_dbg1_fd, __VA_ARGS__)
#else
   #define DBG1_PRINT(...)
#endif

#ifdef DBG_LEVEL_2
   #pragma message "DBG_LEVEL_2 on"
   #ifndef DBG_MODE
      #define DBG_MODE
   #endif
   #define DBG2_PRINT(...) \
      fprintf(__debug_dbg2_fd, __VA_ARGS__)
#else
   #define DBG2_PRINT(...)
#endif

//DBG_PRINT for all levels
#ifdef DBG_MODE
   #pragma message "DBG_MODE on"
   #define DBG_PRINT(...) \
      fprintf(__debug_dbg2_fd, __VA_ARGS__)
#else
   #define DBG_PRINT(...)
#endif

#define ERR_PRINT(...) \
   fprintf(__debug_err_fd, __VA_ARGS__)

#define LOG_PRINT(...) \
   fprintf(__debug_log_fd, __VA_ARGS__)

#endif // DEBUG_H
