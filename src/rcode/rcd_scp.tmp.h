/* template file: rcd_scp.tmp.h

	This file is part of the rcode project.

	Copyright (C) 2019-2022 Tomasz Pawlak,
	e-mail: tomasz.pawlak@wp.eu

	License: GNU Lesser General Public License version 3 (LGPLv3+)

	The rcd_scp.tmp.h is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 3 of the License, or (at your
	option) any later version.

	The rcd_scp.tmp.h is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
	or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
	License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with the rcode project; see the file COPYING. If not,
	see <http://www.gnu.org/licenses/>.
*/

#ifndef %rcd_bname%_RCD_SCP_H
#define %rcd_bname%_RCD_SCP_H 1

#include "%path%rcode.h"

#include <sys/cdefs.h>
__BEGIN_DECLS

// Besides returning the scope pointer, each call to this function
// updates the rcd_scope_t.vmsg pointer.
// The vmsg struct is stored in TLS, so it's pointer can't be initialized statically.
// The alternative would be to store all the scope data in TLS, but this becomes
// problematic if the rcd_msg_t message array is large.
const struct rcd_scope*
%rcd_bname%_rcdGetScopePtr() __attribute__((pure));

/* if scp == NULL, returns *this* scope ptr.
*/
size_t
%rcd_bname%_rcdGetMinMsgBufSz(const struct rcd_scope *const scp);

/* Assembles null-terminated rcode description in the *buff,
   returns length of the string, incl. null byte, -1 for error.
   Resolved to RCD_PRINT_BUF() in basic and dummy modes
   if scp == NULL, use *this* scope ptr.
*/
int
%rcd_bname%_rcdGetMsg(const struct rcd_scope *const scp, rcode retU, char* buf, int bsz)
__nonnull((3));

/* for convenience only: */
#define %rcd_bname%_RCD_GET_MSG(_rcd, _buf, _bsz) \
   %rcd_bname%_rcdGetMsg(%rcd_bname%_rcdGetScopePtr(), (_rcd), (_buf), (_bsz))

/* internal fn, use:
   ($base_name)_RCD_RETURN_FAULT_VMSG() or ($base_name)_RCD_SET_FAULT_VMSG()
   defined below */
void
__%rcd_bname%_rcdSetVmsg(rcode, const char *fmt, ... ) __attribute__(( format(printf, 2, 3) ));


#define %rcd_bname%_RCD_RETURN_FAULT_VMSG( _rcd, _fmt, ... ) \
	{ \
		_rcd = RCD_SET_VAL( RCD_UNIT, RCD_FVMSG, __LINE__ ); \
		__%rcd_bname%_rcdSetVmsg(_rcd, _fmt, ##__VA_ARGS__); \
		return (_rcd); \
	}

/* same as ($base_name)_RCD_RETURN_FAULT_VMSG() - but it doesn't return immediately.
   NOTE: changing the rcode before returning invalidates the message. */
#define %rcd_bname%_RCD_SET_FAULT_VMSG( _rcd, _fmt, ... ) \
	{ \
		_rcd = RCD_SET_VAL( RCD_UNIT, RCD_FVMSG, __LINE__ ); \
		__%rcd_bname%_rcdSetVmsg(_rcd, _fmt, ##__VA_ARGS__); \
	}


__END_DECLS

#endif /* %rcd_bname%_RCD_SCP_H */
