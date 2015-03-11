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
#ifndef _LIBINAC_PCAP_H_
#define _LIBINAC_PCAP_H_

/*
 *
 */


#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ina_pcap_open_mode_e {
	INA_PCAP_OPEN_MODE_FIO,
	INA_PCAP_OPEN_MODE_MMAP
} ina_pcap_open_mode_t;

/* opaque */
typedef struct ina_pcap_ctx_s ina_pcap_ctx_t;

/*
 *
 */
INA_API(ina_rc_t) ina_pcap_open(const char *pcap_file, ina_pcap_open_mode_t mode, ina_pcap_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_pcap_close(ina_pcap_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_pcap_packet_next(ina_pcap_ctx_t *ctx, size_t *packet_len, unsigned char **packet);

/*
 *
 */
INA_API(ina_rc_t) ina_pcap_read_udp_header(ina_pcap_ctx_t *ctx, size_t packet_len, unsigned char *packet, ina_net_udp_hdr_t **udp_hdr);

#ifdef __cplusplus
}
#endif

#endif
