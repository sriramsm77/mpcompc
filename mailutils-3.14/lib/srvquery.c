#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>

#include "mailutils/srvquery.h"

int resolveSRV(const char* host, SRVRecord* resolved) {
	struct __res_state res;
	if (res_ninit(&res) != 0)
		return -1;

	unsigned char answer[PACKETSZ];
	int len = res_nsearch(&res, host, C_IN, T_SRV, answer, sizeof(answer));

	if (len < 0) {
		fprintf(stderr, "res_nsearch: %s\n", hstrerror(h_errno));
		return -1;
	}

	ns_msg handle;
	ns_rr rr;

	ns_initparse(answer, len, &handle);

	for (int i = 0; i < ns_msg_count(handle, ns_s_an); i++) {
		if (ns_parserr(&handle, ns_s_an, i, &rr) < 0 || ns_rr_type(rr) != T_SRV) {
			perror("ns_parserr");
			continue;
		}

		resolved->priority = ns_get16(ns_rr_rdata(rr));
		resolved->weight   = ns_get16(ns_rr_rdata(rr) + NS_INT16SZ);
		resolved->port     = ns_get16(ns_rr_rdata(rr) + 2 * NS_INT16SZ);
		// decompress domain name
		if (dn_expand(ns_msg_base(handle), ns_msg_end(handle), ns_rr_rdata(rr) + 3 * NS_INT16SZ, resolved->dname,
		              sizeof(resolved->dname)) < 0)
			continue;
		return 0;
	}

	return -1;
}

