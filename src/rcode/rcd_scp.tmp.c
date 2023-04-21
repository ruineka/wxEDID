/* rcode template file: rcd_scp.tmp.c */

#include "%rcd_bname%_rcd_scope.h"

__BEGIN_DECLS

// rcd_autogen: full/basic mode

// "volatile"/variable message:
//  valid only right after the rcode is returned
static __thread rcd_vmsg_t __%rcd_bname%_vmsg;

const struct rcd_scope*
%rcd_bname%_rcdGetScopePtr() {
	%rcd_bname%_scope.hdr.vmsg = &__%rcd_bname%_vmsg; //TLS pointer
	return (const struct rcd_scope*) &%rcd_bname%_scope;
}

//full mode (2) -> VMSG defines min buffer size, otherwise
//it's the max length of static message
//the rcd_autogen_md variable is injected by rcd_autogen
//RCD_MODE_BASIC comes from rcode.h
size_t
%rcd_bname%_rcdGetMinMsgBufSz(const struct rcd_scope *scp) {
   if (NULL == scp) {scp = (const struct rcd_scope*) &%rcd_bname%_scope; }
	return (size_t) ( ((const rcd_scope_t*) scp)->hdr.rcdgen_md > RCD_MODE_BASIC) ? RCD_VMSG_MAX_SZ : %rcd_bname%_scope.hdr.min_bufsz;
}

__END_DECLS
