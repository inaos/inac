/*
 * Copyright (c) 2012-2015, INAOS GmbH
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
#ifndef _LIBINAC_NET_H__
#define _LIBINAC_NET_H__

/*
 * INAOS Network API
 */

#include <libinac/lib.h>

#ifdef INA_OS_LINUX
#include <netinet/in.h>
#include <poll.h>
#elif INA_OS_WIN32
#include <winsock.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ina_net_hw_ctx_s ina_net_hw_ctx_t;

typedef enum ina_net_hw_backend_e {
    INA_NET_HW_BACKEND_SOLARFLARE_ONLOAD = 0,
    INA_NET_HW_BACKEND_MELLANOX_VMA,
} ina_net_hw_backend_t;

static const char ina_net_hw_backend_str[][32] = {
    "SOLARFLARE - OPENONLOAD",
    "MELLANOX - VMA"
};

typedef enum ina_net_hw_feature_s {
    INA_NET_HW_FEATURE_ZERO_COPY_UDP_RECEIVE,
    INA_NET_HW_FEATURE_ZERO_COPY_UDP_SEND,
    INA_NET_HW_FEATURE_ZERO_COPY_TCP_RECEIVE,
    INA_NET_HW_FEATURE_ZERO_COPY_TCP_SEND,
    INA_NET_HW_FEATURE_MSG_WARM, /* Keep the TLB and CPU cache warm for infrequent send() */
    INA_NET_HW_FEATURE_TPL_SEND,
    INA_NET_HW_FEATURE_LOOPBACK, /* Accelerate loopback traffic in usermode or hw */
} ina_net_hw_feature_t;

#define INA_NET_ETHER_ADDR_LEN  6

typedef struct ina_net_ether_header_s {
    uint8_t ether_dhost[INA_NET_ETHER_ADDR_LEN];
    uint8_t ether_shost[INA_NET_ETHER_ADDR_LEN];
    uint16_t ether_type;
} INA_PACKED ina_net_ether_header_t;

/*
 * Definitions for internet protocol version 4.
 * Per RFC 791, September 1981.
 */

/*
 * Structure of an internet header, naked of options.
 *
 * We declare ip_len and ip_off to be short, rather than u_short
 * pragmatically since otherwise unsigned comparisons can result
 * against negative integers quite easily, and fail in subtle ways.
 */
typedef struct ina_net_ip_s {
#ifdef INA_LITTLE_ENDIAN
    u_char	ip_hl:4,		/* header length */
        ip_v:4;		        /* version */
#elif INA_BIG_ENDIAN
	u_char	ip_v:4,			/* version */
		ip_hl:4;    		/* header length */
#endif
	u_char	ip_tos;			/* type of service */
	short	ip_len;			/* total length */
	u_short	ip_id;			/* identification */
	short	ip_off;			/* fragment offset field */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
	u_char	ip_ttl;			/* time to live */
	u_char	ip_p;			/* protocol */
	u_short	ip_sum;			/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
} ina_net_ip_t;

/* 
 * Per RFC 768, September, 1981.
 */
typedef struct ina_net_udp_hdr_s {
	u_short uh_sport;		/* source port */
	u_short	uh_dport;		/* destination port */
	u_short	uh_ulen;		/* datagram length */
	u_short	uh_sum;			/* datagram checksum */
} ina_net_udp_hdr_t;

#ifdef INA_OS_WIN32
typedef ULONG nfds_t;
#endif

/*
 * Resovle an host name into to a ip address
 * 
 * Parameters
 * host     Hostname
 * ipbuf    Char buffer for ip address
 *
 * Return Value
 * INA_SUCCES if no error occured.
 */
INA_API(ina_rc_t) ina_net_resolve(const char *host, char *ipbuf);
/*
 *
 */
INA_API(ina_rc_t) ina_net_hostname(char *host, size_t len);
/*
 *
 */
INA_API(ina_rc_t) ina_net_tcp_server(int *fd, int port, const char *bindaddr);

/*
 *
 */
INA_API(ina_rc_t) ina_net_tcp_accept(int *fd, int sfd, char *ip, int *port);

/*
 *
 */
INA_API(ina_rc_t) ina_net_tcp_connect(int *fd, const char *addr, int port, int timeout_sec);

/*
 *
 */
INA_API(ina_rc_t) ina_net_read(int fd, unsigned char *buf, int nb, int* nb_read);

/*
 *
 */
INA_API(ina_rc_t) ina_net_write(int fd, const unsigned char *buf, int nb, int* nb_write);

/*
 *
 */
INA_API(ina_rc_t) ina_net_nonblock(int fd);

/*
 *
 */
INA_API(ina_rc_t) ina_net_block(int fd);

/*
 *
 */
INA_API(ina_rc_t) ina_net_set_read_timeout(int fd, int msec);

/*
 *
 */
INA_API(ina_rc_t) ina_net_set_write_timeout(int fd, int msec);

/*
 *
 */
INA_API(ina_rc_t) ina_net_close(int fd);

/*
 *
 */
INA_API(ina_rc_t) ina_net_udp_bind(int* fd, const char *addr, int port);

/*
 *
 */
INA_API(ina_rc_t) ina_net_join_group(int fd, const char *localif, const char *source);

/*
 *
 */
INA_API(ina_rc_t) ina_net_leave_group(int fd, const char *localif, const char *source);

/*
 *
 */
INA_API(ina_rc_t) ina_net_get_mac_addr(const char *ip, char *mac);

/*
 * Level triggered readiness notification, good enough for a couple of thousend
 * connections and operates only in userspace. When HW accelerated does spin-wait
 * compared to kernel sleep. Timeout is in milliseconds
 *
 */
INA_API(ina_rc_t) ina_net_poll(struct pollfd *fds, nfds_t nfds, int timeout, int *num_fds_ready);

/*
 *
 */
INA_API(int) ina_net_hw_support_present_on_os();

/*
 *
 */
INA_API(ina_rc_t) ina_net_hw_init(ina_net_hw_ctx_t **ctx, ina_net_hw_backend_t backend);

/*
 *
 */
INA_API(ina_rc_t) ina_net_hw_destroy(ina_net_hw_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_net_hw_set_user_data(ina_net_hw_ctx_t *ctx, void *data);

/*
 *
 */
INA_API(ina_rc_t) ina_net_hw_backend_name(ina_net_hw_ctx_t *ctx, ina_str_t *name);

/*
 *
 */
INA_API(ina_rc_t) ina_net_hw_accelerate_loopback(ina_net_hw_ctx_t *ctx, int fd, const char *alias);

/*
 *
 */
INA_API(ina_rc_t) ina_net_hw_feature_check(ina_net_hw_ctx_t *ctx, ina_net_hw_feature_t feature);

#ifdef __cplusplus
}
#endif

#endif
