/* Declarations for rcode_scp.h

   Copyright (C) 2020-2022 Tomasz Pawlak,
   e-mail: tomasz.pawlak@wp.eu

   rcode_scp.h v0.0.1 (2020.11.24)

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

#ifndef RCODE_SCOPE_H
#define RCODE_SCOPE_H 1

#include "rcode.h"

/* included in files generated by rcd_autogen */

#define RCD_VMSG_MAX_SZ 512

#include <sys/cdefs.h>
__BEGIN_DECLS

/* "volatile"/variable message: valid only right after the rcode is returned
   This structure is hold in TLS, see rcd_scp.tmp.c
*/
typedef struct __attribute__ ((packed)) rcd_vmsg_s {
   rcode    retU;
   uint32_t msg_len;
   char     msg_buf[RCD_VMSG_MAX_SZ];
} rcd_vmsg_t;

/* message assigned to a given line in the source file */
typedef struct __attribute__ ((packed)) rcd_msg_s {
   const char     *msg;
   const uint16_t  msg_len;
   const uint16_t  lnum;
} rcd_msg_t;

/* scope descriptor */
typedef struct __attribute__ ((packed)) rcd_unit_s {
   const rcd_msg_t *msg_ar;
   const char      *un_dir;
   const char      *un_file;
   const uint16_t   dir_slen;
   const uint16_t   file_slen;
   const uint16_t   un_id;
   const uint16_t   msg_cnt;
} rcd_unit_t;

/* scope header */
typedef struct __attribute__ ((packed)) rcd_scphdr_s {
   rcd_vmsg_t     *vmsg; /* TLS pointer, see rcd_scp.tmp.c */
   const char     *base_name;
   const uint16_t  bname_slen;
   const uint16_t  unit_cnt;   //unit count
   const uint16_t  min_bufsz;
   const uint8_t   rcdgen_md;  //rcd_autogen run mode
   const uint8_t   reserve;    //alignment
} rcd_scphdr_t;


__END_DECLS

#endif /* RCODE_SCOPE_H */