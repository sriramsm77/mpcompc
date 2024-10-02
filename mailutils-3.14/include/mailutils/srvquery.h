/*
 *
 */
#ifndef __SRV_QUERY_H__
#define __SRV_QUERY_H__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>

typedef struct SRVRecord {
	u_int16_t priority;
	u_int16_t weight;
	u_int16_t port;
	char dname[MAXCDNAME];
} SRVRecord;

int resolveSRV(const char* host, SRVRecord* resolved);

#endif /* __SRV_QUERY_H__ */
