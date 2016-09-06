/*
 * Copyright (c) 2013-2016, INAOS GmbH
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
#ifndef _LIBINAC_DNS_H_
#define _LIBINAC_DNS_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* opaque DNS context */
typedef struct ina_dns_ctx_s ina_dns_ctx_t;

/*
 * Create and initialize a new DNS context.
 *
 * Parameters
 *  ctx  Where to store the newly created context
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_dns_init(ina_dns_ctx_t **ctx);

/*
 * Destroy a DNS context.
 *
 * Parameters
 *  ctx  Context to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_dns_destroy(ina_dns_ctx_t **ctx);

/*
 * Retrieve IPv4 addresses for a given hostname.
 *
 * Parameters
 *  ctx            DNS context
 *  hostname       Hostname to query
 *  address_count  Where to store address count
 *  addresses      Where to store ip addresses
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_dns_system_lookup(ina_dns_ctx_t *ctx,
                                        const char *hostname,
                                        short *address_count,
                                        ina_str_t **addresses);

#ifdef __cplusplus
}
#endif

#endif
