/* Declarations for rcode.h

   Copyright (C) 2013-2022 Tomasz Pawlak,
   e-mail: tomasz.pawlak@wp.eu

   rcode.h v1.1.0 (2020.11.24)

   License: GNU Lesser General Public License version 3 (LGPLv3+)

   The rcode project is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   The rcode project is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with the rcode project; see the file COPYING. If not,
   see <http://www.gnu.org/licenses/>.
*/

/* Short description:
   Compacted return type:
   By default it provides exit status, source file ID (aka unit) and
   the source line number.
   The structure can also be used to return true/false booleans or just
   any user-defined value, as a normal int32 type.

   The 'rcd_autogen' extension allows to handle relative paths, file names and
   textual messages tied to particular rcode values.
*/

#ifndef RCODE_H
#define RCODE_H 1

#ifdef __cplusplus
   #ifndef __STDC_LIMIT_MACROS
      #define __STDC_LIMIT_MACROS
   #endif
#endif

#include <stdint.h>
#include <stdio.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

typedef unsigned int uint;

typedef union __attribute__ ((__transparent_union__)) rcode_u {
   int32_t  value;
   struct __attribute__ ((packed)) detail_s {
      int      rcode :2 ; /* -2 .. 1 */
      uint     unit  :14;
      uint16_t data;
   } detail;
} rcode;

#ifndef RCD_UNIT
   #define RCD_UNIT 0
#endif

/* return codes: */
#define RCD_TRUE  ((int)  1) /* OK/TRUE: RCD_RETURN_TRUE(), RCD_RETURN_TRUE_MSG() */
#define RCD_OK    ((int)  0) /* OK/FALSE: RCD_RETURN_OK(), RCD_RETURN_OK_MSG(),
                                          RCD_RETURN_FALSE(), RCD_RETURN_FALSE_MSG() */
#define RCD_F     ((int) -1) /* FAULT/FALSE: RCD_RETURN_FAULT(), RCD_RETURN_FAULT_MSG() */
#define RCD_FVMSG ((int) -2) /* FAULT/FALSE: VMSG: "volatile" message, used with rcd_autogen */
/* pseudo bools:  */
/*      RCD_TRUE  */
#define RCD_FALSE RCD_OK

/* NOTE:
   rcode.detail.unit == 0 is a reserved value: it means that the rcode is not assigned
   to any particular unit.
   In the result, the allowed unit number range is: 1..16383.
*/
#define RCD_UNIT_NONE ((uint) 0     ) /* reserved unit number */
#define RCD_UNIT_MAX  ((uint) 0x3FFF) /* 16383 */
#define RCD_DATA_MAX  ((uint) 0xFFFF)

/* RCD_UNIT:
   It's an identifier for particular unit file. Together with the line number
   in the .data field it defines exact location of an event in a whole project
   or in a sub-module of a project.

   Therefore, the unit number must be unique within a project or within a module.
   The easiest way to assure this, is to create a shared header file with a list of units:

   rcdunits.h:
    #define idMain     1
    #define idDataBase 2

   then in main.c:
    #include "rcdunits.h"
    #define RCD_UNIT idMain
    #include "rcode/rcode.h"

   database.c:
    #include "rcdunits.h"
    #define RCD_UNIT idDataBase
    #include "rcode/rcode.h"

   To assure that the RCD_UNIT value is the same for the whole
   unit code, the rcode.h must be included before any other header
   which is referencing rcode.h.
*/

/* check pseudo-bools: (true) */
__always_inline
int RCD_IS_TRUE(rcode rcd) {
   return ( rcd.detail.rcode >= RCD_TRUE );
}

/* check pseudo-bools: (false) */
__always_inline
int RCD_IS_FALSE(rcode rcd) {
   return ( rcd.detail.rcode <= RCD_FALSE );
}

/* get unit code from rcode */
__always_inline
uint RCD_GET_UNIT(rcode rcd) {
   return ( rcd.detail.unit );
}

/* get rcode from rcode */
__always_inline
int RCD_GET_RCODE(rcode rcd) {
   return ( rcd.detail.rcode );
}

/* get data from rcode */
__always_inline
uint16_t RCD_GET_DATA(rcode rcd) {
   return ( rcd.detail.data );
}

/* return rcode status: error={RCD_F,RCD_FVMSG}, ok={RCD_OK,RCD_VAL} */
__always_inline
int RCD_IS_OK(rcode rcd) {
   return ( rcd.detail.rcode >= RCD_OK );
}

/* create custom return code */
__always_inline
rcode RCD_SET_VAL(uint unit, int __rc, uint16_t hword) {
   rcode rcd;
   rcd.value = (int32_t) ( (__rc & 0x3) | ((unit & RCD_UNIT_MAX) << 2) | ((hword & RCD_DATA_MAX) << 16) );
   return rcd;
}

/* set status OK */
#define RCD_SET_OK( __rc ) __rc = RCD_SET_VAL( RCD_UNIT, RCD_OK, __LINE__ )

/* set fault code */
#define RCD_SET_FAULT( __rc ) __rc = RCD_SET_VAL( RCD_UNIT, RCD_F, __LINE__ )

#define RCD_RETURN_FAULT( __rc ) { \
   RCD_SET_FAULT( __rc ); \
   return (__rc); }

#define RCD_RETURN_OK( __rc ) { \
   RCD_SET_OK( __rc ); \
   return (__rc); }

#define RCD_RETURN_TRUE( __rc ) { \
   __rc = RCD_SET_VAL( RCD_UNIT, RCD_TRUE, __LINE__ ); \
   return (__rc); }

#define RCD_RETURN_FALSE( __rc ) { \
   __rc = RCD_SET_VAL( RCD_UNIT, RCD_FALSE, __LINE__ ); \
   return (__rc); }

/* RCD_PRINT: print the rcode structure, max str. len=44bytes */
__always_inline
void RCD_PRINT(rcode rcd, FILE* strm) {
   fprintf(strm, "RCODE=%d, unit=%u, data=%u [ 0x%04X ]\n",
      rcd.detail.rcode,
      rcd.detail.unit ,
      rcd.detail.data ,
      rcd.detail.data );
}

#define RCD_PRINT_MIN_BUF_SZ 44
/* print to buffer */
__always_inline
int RCD_PRINT_BUF(rcode rcd, char* buff, int bsz) {
   return
   snprintf(buff, bsz, "RCODE=%d, unit=%u, data=%u [ 0x%04X ]\n",
      rcd.detail.rcode,
      rcd.detail.unit ,
      rcd.detail.data ,
      rcd.detail.data );
}

/* RCD_PRINT for all debug levels */
#ifdef DBG_MODE
   #define RCD_PRINT_DBG( __rc, __fd ) \
      RCD_PRINT( __rc, __fd )
#else
   #define RCD_PRINT_DBG(...)
#endif

//---------------------| RCODE AUTOGEN |---------------------

/* RCD_AUTOGEN_DEFINE_UNIT has to be invoked before any other MSG macros,
   otherwise the rcd_autogen will fail due to undefined unit number
*/
#ifdef _RCD_AUTOGEN
   #define RCD_AUTOGEN_DEFINE_UNIT \
      _RCD_MAGIC_%RCD%UNIT%RCD%RCD_UNIT%RCD%
#else
   #define RCD_AUTOGEN_DEFINE_UNIT
#endif

#define RCD_MODE_DUMMY 0
#define RCD_MODE_BASIC 1
#define RCD_MODE_FULL  2

/* RCD_xxxx_MSG(_rcd, _msg): Those macros are used by rcd_autogen to generate a source file
   with rcd_msg_t, rcd_unit_t and rcd_scope_t structures with unit names, _msg strings and
   corresponding line numbers.
   This information can be later used f.e. by calling <$base_name>_rcdGetMsg(rcode) or
   <$base_name>_rcdPrintMsg(rcode).

   LIMITATION:
   The message string has to be placed on the same line on which the macro is invoked,
   otherwise things will break.
   Multi-line messages or line continuation marks are not directly supported.

   WORKAROUND:
   #define M_MULTILINE_STR \
   "some string, line1"\
   "some string, line2"
   RCD_SET_FAULT_MSG(ret, M_MULTILINE_STR)
*/
#ifdef _RCD_AUTOGEN
   #define RCD_RETURN_FAULT_MSG( _rcd, _msg ) \
      _RCD_MAGIC_%RCD%LN_N%RCD%__LINE__%RCD%_msg%RCD%
#else
   #define RCD_RETURN_FAULT_MSG( _rcd, _msg ) \
   RCD_RETURN_FAULT( _rcd )
#endif

#ifdef _RCD_AUTOGEN
   #define RCD_SET_FAULT_MSG( _rcd, _msg ) \
      _RCD_MAGIC_%RCD%LN_N%RCD%__LINE__%RCD%_msg%RCD%
#else
   #define RCD_SET_FAULT_MSG( _rcd, _msg ) \
   RCD_SET_FAULT( _rcd )
#endif

#ifdef _RCD_AUTOGEN
   #define RCD_RETURN_OK_MSG( _rcd, _msg ) \
      _RCD_MAGIC_%RCD%LN_N%RCD%__LINE__%RCD%_msg%RCD%
#else
   #define RCD_RETURN_OK_MSG( _rcd, _msg ) \
   RCD_RETURN_OK( _rcd )
#endif

#ifdef _RCD_AUTOGEN
   #define RCD_SET_OK_MSG( _rcd, _msg ) \
      _RCD_MAGIC_%RCD%LN_N%RCD%__LINE__%RCD%_msg%RCD%
#else
   #define RCD_SET_OK_MSG( _rcd, _msg ) \
   RCD_SET_OK( _rcd )
#endif

#ifdef _RCD_AUTOGEN
   #define RCD_RETURN_FALSE_MSG( _rcd, _msg ) \
      _RCD_MAGIC_%RCD%LN_N%RCD%__LINE__%RCD%_msg%RCD%
#else
   #define RCD_RETURN_FALSE_MSG( _rcd, _msg ) \
   RCD_RETURN_FALSE( _rcd )
#endif

#ifdef _RCD_AUTOGEN
   #define RCD_RETURN_TRUE_MSG( _rcd, _msg ) \
      _RCD_MAGIC_%RCD%LN_N%RCD%__LINE__%RCD%_msg%RCD%
#else
   #define RCD_RETURN_TRUE_MSG( _rcd, _msg ) \
      RCD_RETURN_TRUE( _rcd )
#endif

/* included in files generated by rcd_autogen */
#ifdef _USE_RCD_AUTOGEN
/* Dummy type declaration, used only for checking of type-correctness of pointers.
   Internal representation is not exposed to client app. */
struct rcd_scope;
#endif

__END_DECLS

#endif /* RCODE_H */
