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
#include <libinac/lib.h>
#include "config.h"

#define LINKTYPE_ETHERNET	1

#define PCAP_MAGIC_SAME_BO          0xa1b2c3d4
#define PCAP_MAGIC_SWAPPED_BO       0xd4c3b2a1
#define PCAP_MAGIC_NSP_SAME_BO      0xa1b23c4d
#define PCAP_MAGIC_NSP_SWAPPED_BO   0x4d3cb2a1 
 
typedef struct __ina_pcap_hdr_s {
	uint32_t magic_number;   /* magic number */
	uint16_t version_major;  /* major version number */
	uint16_t version_minor;  /* minor version number */
	int32_t  thiszone;       /* GMT to local correction */
	uint32_t sigfigs;        /* accuracy of timestamps */
	uint32_t snaplen;        /* max length of captured packets, in octets */
	uint32_t network;        /* data link type */
} __ina_pcap_hdr_t;

typedef struct __ina_pcaprec_hdr_s {
	uint32_t ts_sec;         /* timestamp seconds */
	uint32_t ts_usec;        /* timestamp microseconds */
	uint32_t incl_len;       /* number of octets of packet saved in file */
	uint32_t orig_len;       /* actual length of packet */
} __ina_pcaprec_hdr_t;

struct ina_pcap_ctx_s {
    ina_file_ctx_t *file_ctx;
    ina_file_t *fcapture;
    ina_file_cursor_t *fcur;
    ina_mmap_ctx_t *mmap_ctx;
    __ina_pcap_hdr_t *pcap_hdr;
    int swap_bytes;
    int nano_second_timestamps;
    int first_packet;
};

/** 
 * 
 * https://blogs.oracle.com/DanX/entry/optimizing_byte_swapping_for_fun
 *
 */

INA_API(ina_rc_t) ina_pcap_open(const char *pcap_file, ina_pcap_open_mode_t mode, uint64_t buffer_size, ina_pcap_ctx_t **ctx)
{
    size_t read = 0;
    const unsigned char *chunk;
    uint64_t file_cap_size = 0;
    ina_file_stat_t *fstat;
    uint64_t real_buffer = 0;

    *ctx = (ina_pcap_ctx_t*)ina_mem_alloc(sizeof(ina_pcap_ctx_t));
    (*ctx)->swap_bytes = 0;
    (*ctx)->nano_second_timestamps = 0;
    (*ctx)->first_packet = 1;
    (*ctx)->mmap_ctx = NULL;

    if (!INA_SUCCEED(ina_file_init(&(*ctx)->file_ctx))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_file_new((*ctx)->file_ctx, pcap_file, 
        INA_FILE_ACCESS_MODE_READ, INA_FILE_CREATE_MODE_OPEN, 
        INA_FILE_SHARE_MODE_READ, INA_FILE_FLAG_SEQUENTIAL_ACCESS,
        &(*ctx)->fcapture))) {
            return INA_ERR_PUSH_LAST;
    }

    if (!INA_SUCCEED(ina_file_stat_new((*ctx)->fcapture, &fstat))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_file_stat_file_size(fstat, &file_cap_size))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_file_stat_free((*ctx)->fcapture, &fstat))) {
        return INA_ERR_PUSH_LAST;
    }

    switch (mode) {
        case INA_PCAP_OPEN_MODE_FIO:
            if (!INA_SUCCEED(ina_file_cursor_new((*ctx)->fcapture, INA_FILE_CURSOR_TYPE_FILEIO, 
                INA_FILE_CURSOR_MODE_READ_BINARY, buffer_size, &(*ctx)->fcur, NULL))) {
                    return INA_ERR_PUSH_LAST;
            }
            break;
        case INA_PCAP_OPEN_MODE_MMAP:
            if (!INA_SUCCEED(ina_mmap_init(&(*ctx)->mmap_ctx))) {
                return INA_ERR_PUSH_LAST;
            }
#ifdef INA_CPU_X86_64
            if (buffer_size <= INA_FILE_CURSOR_MMAP_BUFFER_1GB) {
                real_buffer = INA_FILE_CURSOR_MMAP_BUFFER_1GB;
            }
            else if (buffer_size <= INA_FILE_CURSOR_MMAP_BUFFER_2GB) {
                real_buffer = INA_FILE_CURSOR_MMAP_BUFFER_2GB;
            }
            else if (buffer_size <= INA_FILE_CURSOR_MMAP_BUFFER_4GB) {
                real_buffer = INA_FILE_CURSOR_MMAP_BUFFER_4GB;
            }
            else if (buffer_size <= INA_FILE_CURSOR_MMAP_BUFFER_8GB) {
                real_buffer = INA_FILE_CURSOR_MMAP_BUFFER_8GB;
            }
            else if (buffer_size <= INA_FILE_CURSOR_MMAP_BUFFER_16GB) {
                real_buffer = INA_FILE_CURSOR_MMAP_BUFFER_16GB;
            }
            else if (buffer_size <= INA_FILE_CURSOR_MMAP_BUFFER_32GB) {
                real_buffer = INA_FILE_CURSOR_MMAP_BUFFER_32GB;
            }
            else if (buffer_size <= INA_FILE_CURSOR_MMAP_BUFFER_64GB) {
                real_buffer = INA_FILE_CURSOR_MMAP_BUFFER_64GB;
            }
            else {
                real_buffer = INA_FILE_CURSOR_MMAP_BUFFER_128GB;
            }
#else
            real_buffer = INA_FILE_CURSOR_MMAP_BUFFER_1GB;
#endif
            if (!INA_SUCCEED(ina_file_cursor_new((*ctx)->fcapture, INA_FILE_CURSOR_TYPE_MMAP, 
                INA_FILE_CURSOR_MODE_READ_BINARY, real_buffer, &(*ctx)->fcur, (*ctx)->mmap_ctx))) {
                    return INA_ERR_PUSH_LAST;
            }
            break;
        default:
            INA_ASSERT_TRUE(1);
            break;
    }

    /* read pcap header */
    if (!INA_SUCCEED(ina_file_cursor_binary_read_chunk((*ctx)->fcur, sizeof(__ina_pcap_hdr_t), 
        &read, &chunk))) {
            return INA_ERR_PUSH_LAST;
    }

    /* validate pcap header */
    INA_ASSERT_NOTNULL(chunk);
    (*ctx)->pcap_hdr = (__ina_pcap_hdr_t*)ina_mem_alloc(sizeof(__ina_pcap_hdr_t));
    ina_mem_cpy((*ctx)->pcap_hdr, chunk, sizeof(__ina_pcap_hdr_t));

    switch ((*ctx)->pcap_hdr->magic_number) {
        case PCAP_MAGIC_SAME_BO:
            /* this is good no action */
            break;
        case PCAP_MAGIC_SWAPPED_BO:
            (*ctx)->swap_bytes = 1;
            break;
        case PCAP_MAGIC_NSP_SAME_BO:
            (*ctx)->nano_second_timestamps = 1;
            break;
        case PCAP_MAGIC_NSP_SWAPPED_BO:
            (*ctx)->swap_bytes = 1;
            (*ctx)->nano_second_timestamps = 1;
            break;
        default:
            /* this is not a pcap file */
            /* FIXME: proper error-handling */
            INA_ASSERT_TRUE(1);
            return INA_FAILURE;
    }

    /* currently we do not support swapped byte-order */
    if ((*ctx)->swap_bytes) {
        return INA_ENYI;
    }

    /* we only support LINKTYPE_ETHERNET */
    if ((*ctx)->pcap_hdr->network != LINKTYPE_ETHERNET) {
        return INA_ENYI;
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_pcap_close(ina_pcap_ctx_t **ctx)
{
    ina_mem_free((*ctx)->pcap_hdr);
    ina_file_cursor_free(&(*ctx)->fcur);
    if ((*ctx)->mmap_ctx != NULL) {
        ina_mmap_destroy(&(*ctx)->mmap_ctx);
    }
    ina_file_free((*ctx)->file_ctx, &(*ctx)->fcapture);
    ina_file_destroy(&(*ctx)->file_ctx);
    ina_mem_free(*ctx);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_pcap_packet_next(ina_pcap_ctx_t *ctx, size_t *packet_len, unsigned char **packet)
{
    __ina_pcaprec_hdr_t *rec;
    size_t requested = 0;
    size_t read = 0;
    const unsigned char *chunk;

     *packet_len = 0;
     *packet = NULL;

    if (!INA_SUCCEED(ina_file_cursor_binary_read_chunk(ctx->fcur, sizeof(__ina_pcaprec_hdr_t), 
        &read, &chunk))) {
            return INA_ERR_PUSH_LAST;
    }

    /* probably EOF */
    if (read < sizeof(__ina_pcaprec_hdr_t)) {
        return INA_SUCCESS;
    }

    rec = (__ina_pcaprec_hdr_t*)chunk;

    /* do some validation on the first packet */
    INA_ASSERT_TRUE(rec->incl_len == rec->orig_len);
    if (ctx->first_packet) {
        if (rec->incl_len != rec->orig_len) {
            /* FIXME proper error handling */
            return INA_FAILURE;
        }
        ctx->first_packet = 0;
    }

    requested = rec->incl_len;
    if (!INA_SUCCEED(ina_file_cursor_binary_read_chunk(ctx->fcur, requested, 
        &read, &chunk))) {
            return INA_ERR_PUSH_LAST;
    }

    if (read < requested) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }

    *packet_len = read;
    *packet = (unsigned char*)chunk;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_pcap_read_udp_header(ina_pcap_ctx_t *ctx, size_t packet_len, unsigned char *raw_packet, ina_net_udp_hdr_t **udp_hdr)
{
    ina_net_ip_t *ip;
	unsigned int ip_header_length;
    unsigned char *packet = raw_packet;
    size_t pack_len = packet_len;

    *udp_hdr = NULL;

	/* we only support ethernet encapsulation */
	if (pack_len < sizeof(ina_net_ether_header_t)) {
		/* FIXME: proper error handling */
        return INA_FAILURE;
    }

	/* Skip over the Ethernet header. */
	packet += sizeof(ina_net_ether_header_t);
	pack_len -= sizeof(ina_net_ether_header_t);

	if (pack_len < sizeof(ina_net_ip_t)) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }

	ip = (ina_net_ip_t*)packet;
	ip_header_length = ip->ip_hl * 4;	/* ip_hl is in 4-byte words */

	if (pack_len < ip_header_length) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }

	if (ip->ip_p != IPPROTO_UDP) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }

	/* Skip over the IP header to get to the UDP header. */
	packet += ip_header_length;
	pack_len -= ip_header_length;

    if (pack_len < sizeof(ina_net_udp_hdr_t)) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }

	*udp_hdr = (ina_net_udp_hdr_t*)packet;

    return INA_SUCCESS;
}
