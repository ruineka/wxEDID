/* rcode template file: rcd_fn.tmp.c */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "%rcd_bname%_rcd_scope.h"

__BEGIN_DECLS

// rcd_autogen: full mode

#define offsetof(type, member) __builtin_offsetof (type, member)

static char* __rcd_cnv_s(char* buf, uint32_t val) {
// internal fn: args are checked by the caller

	static const char cset[] = "0123456789";
	char   cbuf[16];
	char  *pcbuf = cbuf + sizeof(cbuf);

	do {
		uint32_t utmp = val;
		val /= 10;
		*--pcbuf = cset[utmp - (val * 10)];
	} while (val != 0);

	/* copy buffer */
	while (pcbuf < (cbuf + sizeof(cbuf)) ) {
		*buf++ = *pcbuf++;
	}
	return buf;
}

static const void* __rcd_bin_search(const char *arr, int32_t arsz, uint32_t itemsz, uint32_t voffs, uint32_t val) {
	const char *pitem;
	uint32_t    mval;
	uint32_t    jmp_sz;
	uint32_t    s_cnt;
	 int32_t    idx;

	jmp_sz   = arsz;
	jmp_sz >>= 1;
	idx      = jmp_sz;
	s_cnt    = 0;
	arr     += voffs; //jumping directly between target fields

loop:
	pitem = arr + (idx * itemsz);
	mval  = *((uint16_t*) pitem);
	if (val == mval) { return (pitem - voffs); }
	if (jmp_sz >= 2) {
		jmp_sz >>=1;
	} else { //2 steps left
		if (s_cnt >= 2) goto not_found;
		s_cnt ++ ;
		idx = (val > mval) ? (idx + 1) : (idx - 1);
		if ((idx < 0) || (idx > arsz)) goto not_found;
		goto loop;
	}
	idx = (val > mval) ? (idx + jmp_sz) : (idx - jmp_sz);
	goto loop;

not_found:
	return NULL;
}

static char* __rcd_assemble_msg(char* buf, int32_t bsz, rcode retU,
										const rcd_scope_t *const rscp,
										const rcd_unit_t *p_un,
										const char* msg, uint32_t mlen) {
//min. buff size checked by the caller
//format: "%s[%u]: %s%s.%u [%d] %s": +7 to min_buf size-> autogen::F_APPEND_SORT_UNITS()
	char     *pend;
	uint32_t  len;

	pend  = buf;
	pend += bsz;
	len = rscp->hdr.bname_slen;
	memcpy(buf, rscp->hdr.base_name, (size_t) len); //base name
	buf += len;
	*buf = '['; buf ++ ;
	buf  = __rcd_cnv_s(buf, RCD_GET_UNIT(retU)); //unit number
	*buf = ']'; buf ++ ;
	*buf = ':'; buf ++ ;
	*buf = ' '; buf ++ ;
	len  = p_un->dir_slen;
	memcpy(buf, p_un->un_dir, (size_t) len); //unit dir
	buf += len;
	len  = p_un->file_slen;
	memcpy(buf, p_un->un_file, (size_t) len); //file name
	buf += len;
	*buf = '.'; buf ++ ;
	buf  = __rcd_cnv_s(buf, RCD_GET_DATA(retU)); //ln num
	*buf = ' '; buf ++ ;
	*buf = '['; buf ++ ;
	{ //rcode
		int rcd;
		rcd = RCD_GET_RCODE(retU); // -2 .. +1
		if (rcd < 0) {
			*buf = '-'; buf ++ ;
			rcd  = -rcd;
		}
		rcd += '0';
		*buf = rcd; buf ++ ;
	}
	*buf = ']'; buf ++ ;
	*buf = ' '; buf ++ ;
	if (NULL == msg) goto end;
	len  = (pend - buf);
	if (mlen > len) mlen = len;
	memcpy(buf, msg, (size_t) mlen); //message
	buf += mlen;

end:
	*buf = 0;
	return buf;
}

int %rcd_bname%_rcdGetMsg(const struct rcd_scope *const scp, rcode retU, char* buf, int bsz) {
	const rcd_unit_t  *p_un;
	const char        *msg;
	int32_t            itmp;
	const rcd_scope_t *rscp;

	rscp = (const rcd_scope_t*) scp;

	if (NULL == rscp) rscp = &%rcd_bname%_scope;
	if (0 == RCD_GET_UNIT(retU)) goto fallback; //rcode.detail.unit == 0 -> special case, reserved value.
	if (bsz < rscp->hdr.min_bufsz) goto fallback; //at least rscp->min_bufsz, +RCD_VMSG_MAX_SZ for formatted msg.

	itmp = rscp->hdr.unit_cnt;
	p_un = (rcd_unit_t*) &rscp->unit_ar[0];
//search units
	p_un = (rcd_unit_t*) __rcd_bin_search((char*) p_un, itmp,
												sizeof(rcd_unit_t),
												offsetof(rcd_unit_t, un_id),
												RCD_GET_UNIT(retU) );
	if ( NULL == p_un ) goto fallback; //unit not found
//get message
	msg = NULL;
	if ( RCD_GET_RCODE(retU) != RCD_FVMSG ) {
		rcd_msg_t *pmsg;
//search messages
		itmp = p_un->msg_cnt;
		pmsg = (rcd_msg_t*) &p_un->msg_ar[0];
		if ( NULL == pmsg ) goto no_msg; //no msg in the unit

		pmsg = (rcd_msg_t*) __rcd_bin_search((char*) pmsg, itmp,
													sizeof(rcd_msg_t),
													offsetof(rcd_msg_t, lnum),
													RCD_GET_DATA(retU) );
		if ( NULL == pmsg ) goto no_msg; //no msg for a given line, itmp=msg_len has no meaning in such case.
		msg  = pmsg->msg;
		itmp = pmsg->msg_len;

	} else { //RCD_FVMSG
		rcd_vmsg_t *vmsg;
//NOTE: vmsg is initialized by calling  $(base_name)_rcdGetScopePtr()
		vmsg = rscp->hdr.vmsg;
//check if vmsg matches *this* rcode
		if (retU.value == vmsg->retU.value) {
			msg  = vmsg->msg_buf;
			itmp = vmsg->msg_len;
		}
	}

no_msg:
	{
		char *pend;

		pend = __rcd_assemble_msg( buf, bsz, retU, rscp, p_un, msg, itmp);

		itmp = (pend - buf);
		return itmp;
	}
fallback:
	return RCD_PRINT_BUF(retU, buf, bsz);
}

void __%rcd_bname%_rcdSetVmsg(rcode rcd, const char *fmt, ... ) {
	rcd_vmsg_t *vmsg;
	int         len;
	va_list     argp;
//*this* unit
	vmsg = ( (const rcd_scope_t*) %rcd_bname%_rcdGetScopePtr() )->hdr.vmsg;

	va_start(argp, fmt);

	len = vsnprintf(vmsg->msg_buf, RCD_VMSG_MAX_SZ, fmt, argp );

	va_end(argp);

	vmsg->msg_len = len;
	vmsg->retU    = rcd;
}

__END_DECLS
