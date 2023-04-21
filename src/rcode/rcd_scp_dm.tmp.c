/* rcode template file: rcd_scp_dm.tmp.c */

// rcd_autogen: dummy mode

__BEGIN_DECLS

const struct rcd_scope*
%rcd_bname%_rcdGetScopePtr() {
	return NULL;
}

//rcd_autogen_md in dummy mode is zero
size_t
%rcd_bname%_rcdGetMinMsgBufSz() {
	return (size_t) 0;
}

__END_DECLS
