/*
 * Copyright (c) 2012-2014, INAOS GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the INAOS GmbH nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <libinac/lib.h>
#include "config.h"

#ifndef INA_OS_WIN32
#include <netdb.h>
#include <arpa/inet.h>
#endif

struct ina_dns_ctx_s {
    char dummy;
};

INA_API(ina_rc_t) ina_dns_init(ina_dns_ctx_t **ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    *ctx = (ina_dns_ctx_t*)ina_mem_alloc(sizeof(ina_dns_ctx_t));
    if (*ctx == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dns_destroy(ina_dns_ctx_t **ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    if (*ctx != NULL) {
        ina_mem_free(*ctx);   
    }
    *ctx = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dns_system_lookup(ina_dns_ctx_t *ctx, const char* hostname, short *address_count, ina_str_t **addresses)
{
    short i, cnt;
    struct hostent *remote_host;
    ina_str_t *addresses_ptr;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(hostname);
    INA_ASSERT_NOTNULL(address_count);
    INA_ASSERT_NOTNULL(addresses)

    remote_host = gethostbyname(ina_str_cstr(hostname));
    if (remote_host == NULL || remote_host->h_addrtype != AF_INET) {
        return INA_DNS_ELOOKUP;
    }

    /* count how many addresses we have */
    cnt = 0;
    while (remote_host->h_addr_list[cnt] != 0) {
        cnt++;
    }
    if (cnt == 0) {
        return INA_DNS_ELOOKUP;
    }
    
    *addresses = (ina_str_t*)ina_mem_alloc(sizeof(char)*15*cnt);
    if (addresses == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    addresses_ptr = *addresses;

    for (i = 0; i < cnt; i++) {
        struct in_addr addr;
        addr.s_addr = *(u_long *)remote_host->h_addr_list[i];
        addresses_ptr[i] = ina_str_new_fromcstr(inet_ntoa(addr));
    }

    *address_count = cnt;

    return INA_SUCCESS;
}
