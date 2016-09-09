/*
 * Copyright (c) 2015-2016, INAOS GmbH
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

#define PCAP_VLAN_TAGGING_MAGIC     0x8100

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

typedef ina_rc_t (*__ina_pcap_read_chunk_fp)(ina_pcap_ctx_t *ctx, size_t how_much, size_t *read, const unsigned char **chunk);

struct ina_pcap_ctx_s {
    ina_file_ctx_t *file_ctx;
    ina_file_t *fcapture;
    ina_file_cursor_t *fcur;
    ina_mmap_ctx_t *mmap_ctx;
    ina_gzip_file_t *gzip_ctx;
    unsigned char *gzip_buffer;
    __ina_pcap_hdr_t *pcap_hdr;
    int swap_bytes;
    int nano_second_timestamps;
    int first_packet;
    __ina_pcap_read_chunk_fp read_chunk_fp;
};

/** 
 * 
 * https://blogs.oracle.com/DanX/entry/optimizing_byte_swapping_for_fun
 *
 */

static ina_rc_t __ina_pcap_read_chunk_cursor(ina_pcap_ctx_t *ctx, size_t how_much, size_t *read, const unsigned char **chunk)
{
    return ina_file_cursor_binary_read_chunk(ctx->fcur, how_much, read, chunk);
}

static ina_rc_t __ina_pcap_read_chunk_gzip(ina_pcap_ctx_t *ctx, size_t how_much, size_t *read, const unsigned char **chunk)
{
    ina_rc_t rc = INA_FAILURE;
    unsigned char *orig = ctx->gzip_buffer;
    size_t tot_read = 0;
    *read = 0;
    while (tot_read < how_much) {
        rc = ina_gzip_read_next_block(ctx->gzip_ctx, how_much-tot_read, read, &ctx->gzip_buffer);
        if (*read == 0) {
            break;
        }
        tot_read += *read;
        ctx->gzip_buffer += *read;
    }
    ctx->gzip_buffer = orig;
    *chunk = ctx->gzip_buffer;
    *read = tot_read;
    return rc;
}

static void __ina_pcap_detect_compression(const char *pcap_file,
                                          ina_pcap_file_compression_t *compression)
{
    ina_gzip_file_t *f;

    *compression = INA_PCAP_FILE_COMPRESSION_NONE;

    if (INA_SUCCEED(ina_gzip_open(pcap_file, 4096, &f))) {
        *compression = INA_PCAP_FILE_COMPRESSION_GZIP;
    }
    ina_gzip_close(&f);
}

INA_API(ina_rc_t) ina_pcap_open(const char *pcap_file, ina_pcap_open_mode_t mode,
                                size_t buffer_size,
                                ina_pcap_file_compression_t compression,
                                ina_pcap_ctx_t **ctx)
{
    size_t read = 0;
    const unsigned char *chunk;
    uint64_t file_cap_size = 0;
    ina_file_stat_t *fstat;
    uint64_t real_buffer = 0;

    INA_ASSERT_NOTNULL(pcap_file);
    INA_ASSERT_TRUE(strlen(pcap_file));

    if (compression == INA_PCAP_FILE_COMPRESSION_DETECT) {
        __ina_pcap_detect_compression(pcap_file, &compression);
    }

    if (mode == INA_PCAP_OPEN_MODE_AUTO) {
        if (compression == INA_PCAP_FILE_COMPRESSION_NONE) {
            mode = INA_PCAP_OPEN_MODE_FIO;
        } else {
            mode = INA_PCAP_OPEN_MODE_FIO;
        }
    }

    if (compression == INA_PCAP_FILE_COMPRESSION_NONE &&
            mode == INA_PCAP_OPEN_MODE_MMAP) {
        return INA_FAILURE;
    }

    *ctx = (ina_pcap_ctx_t*)ina_mem_alloc(sizeof(ina_pcap_ctx_t));
    (*ctx)->swap_bytes = 0;
    (*ctx)->nano_second_timestamps = 0;
    (*ctx)->first_packet = 1;
    (*ctx)->mmap_ctx = NULL;
    (*ctx)->gzip_ctx = NULL;
    (*ctx)->gzip_buffer = NULL;
    (*ctx)->fcur = NULL;
    (*ctx)->file_ctx = NULL;
    (*ctx)->fcapture = NULL;

    if (compression == INA_PCAP_FILE_COMPRESSION_NONE) {
        if (!INA_SUCCEED(ina_file_init(&(*ctx)->file_ctx, 0))) {
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
        (*ctx)->read_chunk_fp = __ina_pcap_read_chunk_cursor;
    }
    else {
        if (!INA_SUCCEED(ina_gzip_open(pcap_file, buffer_size, &(*ctx)->gzip_ctx))) {
            return INA_ERR_PUSH_LAST;
        }
        (*ctx)->gzip_buffer = (unsigned char*)ina_mem_alloc(sizeof(unsigned char)*buffer_size);
        (*ctx)->read_chunk_fp = __ina_pcap_read_chunk_gzip;
    }

    /* read pcap header */
    if (!INA_SUCCEED((*ctx)->read_chunk_fp(*ctx, sizeof(__ina_pcap_hdr_t), 
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
    if ((*ctx)->fcur != NULL) {
        ina_file_cursor_free(&(*ctx)->fcur);
    }
    if ((*ctx)->mmap_ctx != NULL) {
        ina_mmap_destroy(&(*ctx)->mmap_ctx);
    }
    if ((*ctx)->gzip_buffer != NULL) {
        ina_mem_free((*ctx)->gzip_buffer);
    }
    if ((*ctx)->gzip_ctx != NULL) {
        ina_gzip_close(&(*ctx)->gzip_ctx);
    }
    if ((*ctx)->fcapture != NULL) {
        ina_file_free((*ctx)->file_ctx, &(*ctx)->fcapture);
    }
    if ((*ctx)->file_ctx != NULL) {
        ina_file_destroy(&(*ctx)->file_ctx);
    }
    ina_mem_free(*ctx);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_pcap_get_open_mode(ina_pcap_ctx_t *pcap,
                                         ina_pcap_open_mode_t *open_mode)
{
    INA_ASSERT_NOTNULL(pcap);
    INA_ASSERT_NOTNULL(open_mode);
    if (pcap->mmap_ctx != NULL) {
        *open_mode = INA_PCAP_OPEN_MODE_MMAP;
    } else {
        *open_mode = INA_PCAP_OPEN_MODE_FIO;
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_pcap_get_file_compression(ina_pcap_ctx_t *pcap,
                                           ina_pcap_file_compression_t *file_compression)
{
    INA_ASSERT_NOTNULL(pcap);
    INA_ASSERT_NOTNULL(file_compression);
    if (pcap->gzip_ctx != NULL) {
        *file_compression = INA_PCAP_FILE_COMPRESSION_GZIP;
    } else {
        *file_compression = INA_PCAP_FILE_COMPRESSION_NONE;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_pcap_strip_vlan(ina_pcap_ctx_t *ctx, size_t *packet_len, unsigned char **raw_packet)
{
    unsigned char *packet = *raw_packet;

    /* VLAN tagged ethernet frame (https://en.wikipedia.org/wiki/IEEE_802.1Q) */
    if (packet[12] == 8 && packet[13] == 0) {
        memmove(&packet[12], &packet[16], *packet_len - 16);
    }
    *raw_packet = packet;
    *packet_len = *packet_len - 4;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_pcap_packet_next(ina_pcap_ctx_t *ctx, size_t *packet_len, unsigned char **packet,
                                       uint64_t *ts_micros)
{
    __ina_pcaprec_hdr_t *rec;
    size_t requested = 0;
    size_t read = 0;
    const unsigned char *chunk;

     *packet_len = 0;
     *packet = NULL;

    if (!INA_SUCCEED(ctx->read_chunk_fp(ctx, sizeof(__ina_pcaprec_hdr_t), 
        &read, &chunk))) {
            return INA_ERR_PUSH_LAST;
    }

    /* probably EOF */
    if (read < sizeof(__ina_pcaprec_hdr_t)) {
        return INA_SUCCESS;
    }

    rec = (__ina_pcaprec_hdr_t*)chunk;

    /* do some validation on the first packet */
    if (ctx->first_packet) {
        if (rec->incl_len != rec->orig_len) {
            /* FIXME proper error handling */
            return INA_FAILURE;
        }
        ctx->first_packet = 0;
    }
    /* Included len should never become larger than orig_len or the snaplen value of the global header. */
    INA_ASSERT_TRUE(rec->incl_len <= rec->orig_len);
    INA_ASSERT_TRUE(rec->incl_len <= ctx->pcap_hdr->snaplen);

    *ts_micros = (rec->ts_sec*1000ULL*1000ULL) + rec->ts_usec;
    requested = rec->incl_len;
    if (!INA_SUCCEED(ctx->read_chunk_fp(ctx, requested, 
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

INA_API(ina_rc_t) ina_pcap_read_headers(ina_pcap_ctx_t *ctx, size_t packet_len, unsigned char *raw_packet, 
                                        ina_net_ip_t **ip_hdr, ina_net_udp_hdr_t **udp_hdr)
{
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

	*ip_hdr = (ina_net_ip_t*)packet;
	ip_header_length = (*ip_hdr)->ip_hl * 4;	/* ip_hl is in 4-byte words */

	if (pack_len < ip_header_length) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }

	if ((*ip_hdr)->ip_p != IPPROTO_UDP) {
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
