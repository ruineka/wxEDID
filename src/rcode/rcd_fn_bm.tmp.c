/* rcode template file: rcd_fn_bm.tmp.c */

// rcd_autogen: basic/dummy mode

__BEGIN_DECLS

#pragma GCC diagnostic ignored "-Wunused-parameter"

int
%rcd_bname%_rcdGetMsg(const struct rcd_scope *const scp, rcode rcd, char *buf, int bsz) {
	return RCD_PRINT_BUF(rcd, buf, bsz);
}

void
__%rcd_bname%_rcdSetVmsg(rcode rcd, const char *fmt, ... ) {
	return;
}

#pragma GCC diagnostic warning "-Wunused-parameter"

__END_DECLS
