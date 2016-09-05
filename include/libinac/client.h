/*
 * Copyright (c) 2015, INAOS GmbH
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
 
#ifndef _LIBINAC_CLIENT_H_
#define _LIBINAC_CLIENT_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DESIGN considerations:
 * + network client
 * + budget oriented
 * + Using poll to dispatch file and network I/O
 * + If high-throughput or low-latency is required we need a
 *   HW accelerated component that dispatches via shared-memory
 * + Use timer for time based tasks
 * + Everything should be mempool based
 * + Pull API to make it ljit friendly
 * + Should provide a simple HTTP(S)-Client  
 *
 */

/* opaque client context */
typedef struct ina_client_ctx_s ina_client_ctx_t;

typedef struct ina_client_descriptor_s {
    uint8_t timer_use_rdtsc;
    uint8_t hw_acceleration;
    ina_net_hw_backend_t hw_backend;
    size_t max_memory_utilization;
} ina_client_descriptor_t;

/*
 *
 */
INA_API(ina_rc_t) ina_client_tcp_new(ina_client_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_client_tcp_free(ina_client_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_client_udp_new(ina_client_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_client_udp_free(ina_client_ctx_t **ctx);

#ifdef __cplusplus
}
#endif

#endif

