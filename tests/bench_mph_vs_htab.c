/*
 * Copyright (c) 2017, INAOS GmbH
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

#ifdef INA_OS_WIN32
#include <Ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#endif

#include <stdlib.h>
#include <libinac/lib.h>

/* 
 * This benchmark stems from the practical application 
 * of parsing pcap files with UDP multicast traffic, 
 * where its important to filter packets.
 * 
 * The 'naive' way of doing this is to build a hashtable 
 * with the ip/port combination that we're intersted in.
 * During playback we will than do a hashtable lookup to 
 * evaluate whether we want to process the packet or not.
 *
 * We benchmark this 'navie' approach with a more specialisted 
 * implementation. Because we know all possible ip/port combinations 
 * for the practical problem we can build a 'minimal perfect hash' function
 * and hook that to a callback table, therefore omitting a general-purpose 
 * hashtable and branching.
 *
 * We test-data is a collection of all possible combinations 
 * of ip/ports that we iterate through a couple of million times in 
 * random order.
 *
 */

static const char ip_port[][32] = {
    "224.0.50.0:59098",
    "224.0.50.128:59098",
    "224.0.50.1:59099",
    "224.0.50.129:59099",
    "224.0.50.7:59001",
    "224.0.50.135:59001",
    "224.0.50.6:59000",
    "224.0.50.134:59000",
    "224.0.114.33:59001",
    "224.0.114.65:59001",
    "224.0.114.32:59000",
    "224.0.114.64:59000",
    "224.0.50.59:59001",
    "224.0.50.187:59001",
    "224.0.50.58:59000",
    "224.0.50.186:59000",
    "224.0.114.35:59001",
    "224.0.114.67:59001",
    "224.0.114.34:59000",
    "224.0.114.66:59000",
    "224.0.50.57:59001",
    "224.0.50.185:59001",
    "224.0.50.56:59000",
    "224.0.50.184:59000",
    "224.0.114.37:59001",
    "224.0.114.69:59001",
    "224.0.114.36:59000",
    "224.0.114.68:59000",
    "224.0.50.55:59001",
    "224.0.50.183:59001",
    "224.0.50.54:59000",
    "224.0.50.182:59000",
    "224.0.114.39:59001",
    "224.0.114.71:59001",
    "224.0.114.38:59000",
    "224.0.114.70:59000",
    "224.0.50.7:59033",
    "224.0.50.135:59033",
    "224.0.50.6:59032",
    "224.0.50.134:59032",
    "224.0.50.5:59001",
    "224.0.50.133:59001",
    "224.0.50.4:59000",
    "224.0.50.132:59000",
    "224.0.114.55:59001",
    "224.0.114.87:59001",
    "224.0.114.54:59000",
    "224.0.114.86:59000",
    "224.0.50.13:59001",
    "224.0.50.141:59001",
    "224.0.50.12:59000",
    "224.0.50.140:59000",
    "224.0.114.41:59001",
    "224.0.114.73:59001",
    "224.0.114.40:59000",
    "224.0.114.72:59000",
    "224.0.50.17:59001",
    "224.0.50.145:59001",
    "224.0.50.16:59000",
    "224.0.50.144:59000",
    "224.0.114.51:59001",
    "224.0.114.83:59001",
    "224.0.114.50:59000",
    "224.0.114.82:59000",
    "224.0.50.15:59033",
    "224.0.50.143:59033",
    "224.0.50.14:59032",
    "224.0.50.142:59032",
    "224.0.114.43:59033",
    "224.0.114.75:59033",
    "224.0.114.42:59032",
    "224.0.114.74:59032",
    "224.0.50.17:59033",
    "224.0.50.145:59033",
    "224.0.50.16:59032",
    "224.0.50.144:59032",
    "224.0.114.51:59033",
    "224.0.114.83:59033",
    "224.0.114.50:59032",
    "224.0.114.82:59032",
    "224.0.50.15:59001",
    "224.0.50.143:59001",
    "224.0.50.14:59000",
    "224.0.50.142:59000",
    "224.0.50.23:59001",
    "224.0.50.151:59001",
    "224.0.50.22:59000",
    "224.0.50.150:59000",
    "224.0.114.45:59001",
    "224.0.114.77:59001",
    "224.0.114.44:59000",
    "224.0.114.76:59000",
    "224.0.50.21:59001",
    "224.0.50.149:59001",
    "224.0.50.20:59000",
    "224.0.50.148:59000",
    "224.0.114.47:59001",
    "224.0.114.79:59001",
    "224.0.114.46:59000",
    "224.0.114.78:59000",
    "224.0.114.57:59001",
    "224.0.114.89:59001",
    "224.0.114.56:59000",
    "224.0.114.88:59000",
    "224.0.50.61:59001",
    "224.0.50.189:59001",
    "224.0.50.60:59000",
    "224.0.50.188:59000",
    "224.0.114.43:59001",
    "224.0.114.75:59001",
    "224.0.114.42:59000",
    "224.0.114.74:59000",
    "224.0.50.19:59001",
    "224.0.50.147:59001",
    "224.0.50.18:59000",
    "224.0.50.146:59000",
    "224.0.114.49:59001",
    "224.0.114.81:59001",
    "224.0.114.48:59000",
    "224.0.114.80:59000",
    "224.0.50.35:59001",
    "224.0.50.163:59001",
    "224.0.50.34:59000",
    "224.0.50.162:59000",
    "224.0.114.53:59001",
    "224.0.114.85:59001",
    "224.0.114.52:59000",
    "224.0.114.84:59000",
    "224.0.50.9:59033",
    "224.0.50.137:59033",
    "224.0.50.8:59032",
    "224.0.50.136:59032",
    "224.0.50.25:59033",
    "224.0.50.153:59033",
    "224.0.50.24:59032",
    "224.0.50.152:59032",
    "224.0.50.35:59033",
    "224.0.50.163:59033",
    "224.0.50.34:59032",
    "224.0.50.162:59032",
    "224.0.50.5:59033",
    "224.0.50.133:59033",
    "224.0.50.4:59032",
    "224.0.50.132:59032",
    "224.0.50.27:59001",
    "224.0.50.155:59001",
    "224.0.50.26:59000",
    "224.0.50.154:59000",
    "224.0.50.29:59033",
    "224.0.50.157:59033",
    "224.0.50.28:59032",
    "224.0.50.156:59032",
    "224.0.50.33:59033",
    "224.0.50.161:59033",
    "224.0.50.32:59032",
    "224.0.50.160:59032",
    "224.0.50.31:59033",
    "224.0.50.159:59033",
    "224.0.50.30:59032",
    "224.0.50.158:59032",
    "224.0.50.49:59033",
    "224.0.50.177:59033",
    "224.0.50.48:59032",
    "224.0.50.176:59032",
    "224.0.50.47:59033",
    "224.0.50.175:59033",
    "224.0.50.46:59032",
    "224.0.50.174:59032",
    "224.0.50.45:59033",
    "224.0.50.173:59033",
    "224.0.50.44:59032",
    "224.0.50.172:59032",
    "224.0.50.51:59033",
    "224.0.50.179:59033",
    "224.0.50.50:59032",
    "224.0.50.178:59032",
    "224.0.50.37:59033",
    "224.0.50.165:59033",
    "224.0.50.36:59032",
    "224.0.50.164:59032",
    "224.0.50.43:59033",
    "224.0.50.171:59033",
    "224.0.50.42:59032",
    "224.0.50.170:59032",
    "224.0.50.41:59033",
    "224.0.50.169:59033",
    "224.0.50.40:59032",
    "224.0.50.168:59032",
    "224.0.50.53:59033",
    "224.0.50.181:59033",
    "224.0.50.52:59032",
    "224.0.50.180:59032",
    "224.0.50.39:59033",
    "224.0.50.167:59033",
    "224.0.50.38:59032",
    "224.0.50.166:59032",
    "224.0.50.3:59033",
    "224.0.50.131:59033",
    "224.0.50.2:59032",
    "224.0.50.130:59032",
    "224.0.50.11:59001",
    "224.0.50.139:59001",
    "224.0.50.10:59000",
    "224.0.50.138:59000",
    "224.0.50.11:59033",
    "224.0.50.139:59033",
    "224.0.50.10:59032",
    "224.0.50.138:59032"
};

#define __INAFX_FAST_EUREX_RDI_SNAPSHOT_P_ADDR       "224.0.50.0"
#define __INAFX_FAST_EUREX_RDI_SNAPSHOT_P_PORT       59098
#define __INAFX_FAST_EUREX_RDI_SNAPSHOT_S_ADDR       "224.0.50.128"
#define __INAFX_FAST_EUREX_RDI_SNAPSHOT_S_PORT       59098
#define __INAFX_FAST_EUREX_RDI_INCREMENTAL_P_ADDR    "224.0.50.1"
#define __INAFX_FAST_EUREX_RDI_INCREMENTAL_P_PORT    59099
#define __INAFX_FAST_EUREX_RDI_INCREMENTAL_S_ADDR    "224.0.50.129"
#define __INAFX_FAST_EUREX_RDI_INCREMENTAL_S_PORT    59099

typedef ina_rc_t (*__inafx_fast_process_packet_cb_t)(struct sockaddr_in *multicast_dst);

typedef struct __inafx_fast_filter_entry_s {
    uint32_t key;
    __inafx_fast_process_packet_cb_t process_cb;
    UT_hash_handle hh;
} __inafx_fast_filter_entry_t;

static ina_stopwatch_t    *stopwatch = NULL;
static ina_str_t           benchmark = NULL;
static struct sockaddr_in *headers = NULL;
static size_t              emdi_counter = 0;
static size_t              rdi_counter = 0;
static size_t              packet_counter = 0;

static __inafx_fast_filter_entry_t *__mph_filter = NULL;
static __inafx_fast_filter_entry_t *__tab_filter = NULL;

/* BEGIN - EUREX PROD mulicast group filter */

/* Range any input might map to = 256 */

static uint32_t __eurex_filter_prod_hash_tab[] = {
    0,45,0,0,0,0,146,0,0,92,113,125,0,0,183,22,
    184,0,113,111,0,0,125,113,183,0,82,82,135,0,253,0,
    183,253,0,0,183,235,0,146,22,61,0,0,235,0,0,61,
    159,113,0,0,58,7,124,0,60,46,0,0,220,184,88,0,
    0,0,0,220,0,8,184,11,0,7,229,125,0,0,167,0,
    32,0,242,24,0,0,185,211,165,0,221,47,0,0,168,191,
    125,114,0,0,100,0,0,18,42,245,0,0,218,183,0,40,
    240,116,0,0,183,74,168,0,255,168,0,0,131,214,12,0,
};

static uint32_t __inafx_fast_eurex_prod_hash(uint32_t val)
{
    uint32_t a, b, rsl;
    val += 0xb1a0f135;
    val ^= (val >> 16);
    val += (val << 8);
    val ^= (val >> 4);
    b = (val >> 14) & 0x7f;
    a = (val + (val << 6)) >> 25;
    rsl = (a^__eurex_filter_prod_hash_tab[b]);
    return rsl;
}

/* END - EUREX PROD mulicast group filter */

static ina_rc_t __inafx_fast_process_rdi_packet(struct sockaddr_in *multicast_dst)
{
    rdi_counter++;
    return INA_SUCCESS;
}

static ina_rc_t __inafx_fast_process_emdi_packet(struct sockaddr_in *multicast_dst)
{
    emdi_counter++;
    return INA_SUCCESS;
}

static ina_rc_t __inafx_fast_process_noop_packet(struct sockaddr_in *multicast_dst)
{
    return INA_SUCCESS;
}

static ina_rc_t __inafx_fast_eurex_filter_create(__inafx_fast_filter_entry_t **head)
{
    int i;
    *head = (__inafx_fast_filter_entry_t*)ina_mem_alloc(sizeof(__inafx_fast_filter_entry_t)*256);
    for (i = 0; i < 256; i++) {
        (*head)[i].process_cb = __inafx_fast_process_noop_packet;
    }
    return INA_SUCCESS;
}

static ina_rc_t __inafx_fast_eurex_filter_key_from_pair(const char *multicast_group, int multicast_port, uint32_t *key_ptr)
{
    struct in_addr as;
    uint8_t third_occ = 0;
    uint8_t fourth_occ = 0;
    unsigned char kb[4];
    uint32_t key, i1, i2, i3;
    uint16_t sport = htons(multicast_port);

#ifdef WIN32
    InetPton(AF_INET, multicast_group, &as);
#else
    inet_aton(multicast_group, &as);
#endif

    third_occ = (as.s_addr >> 8*2) & 0xFF;
    fourth_occ = (as.s_addr >> 8*3) & 0xFF;

    kb[0] = third_occ;
    kb[1] = fourth_occ;
    kb[2] = (sport >> 8*0) & 0xFF;
    kb[3] = (sport >> 8*1) & 0xFF;

    key = kb[0];
    i1 = kb[1];
    i2 = kb[2];
    i3 = kb[3];
    key |= i1 << 8;
    key |= i2 << 16;
    key |= i3 << 24;

    *key_ptr = key;
    return INA_SUCCESS;
}

static ina_rc_t __inafx_fast_eurex_filter_add(const char *multicast_group, 
                                              int multicast_port, 
                                              __inafx_fast_process_packet_cb_t process_cb)
{
    uint32_t key = 0;
    uint32_t idx = 0;

    INA_MUST_SUCCEED(__inafx_fast_eurex_filter_key_from_pair(multicast_group, multicast_port, &key));
    idx = __inafx_fast_eurex_prod_hash(key);

    __mph_filter[idx].key = key;
    __mph_filter[idx].process_cb = process_cb;

    return INA_SUCCESS;
}

static ina_rc_t __inafx_fast_eurex_filter_apply(struct sockaddr_in *multicast_dst, __inafx_fast_filter_entry_t **filter)
{
    unsigned char kb[4];
    uint8_t third_occ = 0;
    uint8_t fourth_occ = 0;
    uint32_t key, i1, i2, i3;
    uint32_t idx = 0;

    third_occ = (multicast_dst->sin_addr.S_un.S_addr >> 8*2) & 0xFF;
    fourth_occ = (multicast_dst->sin_addr.S_un.S_addr >> 8*3) & 0xFF;

    kb[0] = third_occ;
    kb[1] = fourth_occ;
    kb[2] = (multicast_dst->sin_port >> 8*0) & 0xFF;
    kb[3] = (multicast_dst->sin_port >> 8*1) & 0xFF;

    key = kb[0];
    i1 = kb[1];
    i2 = kb[2];
    i3 = kb[3];
    key |= i1 << 8;
    key |= i2 << 16;
    key |= i3 << 24;

    idx = __inafx_fast_eurex_prod_hash(key);
    *filter = &__mph_filter[idx];

    return INA_SUCCESS;
}

/* -------------------------------------------------------------- */

static ina_rc_t __inafx_hash_tab_filter_create(__inafx_fast_filter_entry_t **head)
{
    return INA_SUCCESS;
}

static ina_rc_t __inafx_hash_tab_filter_add(const char *multicast_group, 
                                              int multicast_port, 
                                              __inafx_fast_process_packet_cb_t process_cb)
{
    uint32_t key = 0;
    uint32_t idx = 0;
    __inafx_fast_filter_entry_t *f = (__inafx_fast_filter_entry_t*)ina_mem_alloc(sizeof(__inafx_fast_filter_entry_t));

    f->key = key;
    f->process_cb = process_cb;

    INA_MUST_SUCCEED(__inafx_fast_eurex_filter_key_from_pair(multicast_group, multicast_port, &key));
    HASH_ADD_INT(__tab_filter, key, f);

    return INA_SUCCESS;
}

static ina_rc_t __inafx_hash_tab_filter_apply(struct sockaddr_in *multicast_dst, __inafx_fast_filter_entry_t **filter)
{
    unsigned char kb[4];
    uint8_t third_occ = 0;
    uint8_t fourth_occ = 0;
    uint32_t key, i1, i2, i3;
    __inafx_fast_filter_entry_t *f;

    third_occ = (multicast_dst->sin_addr.S_un.S_addr >> 8*2) & 0xFF;
    fourth_occ = (multicast_dst->sin_addr.S_un.S_addr >> 8*3) & 0xFF;

    kb[0] = third_occ;
    kb[1] = fourth_occ;
    kb[2] = (multicast_dst->sin_port >> 8*0) & 0xFF;
    kb[3] = (multicast_dst->sin_port >> 8*1) & 0xFF;

    key = kb[0];
    i1 = kb[1];
    i2 = kb[2];
    i3 = kb[3];
    key |= i1 << 8;
    key |= i2 << 16;
    key |= i3 << 24;

    HASH_FIND_INT(__tab_filter, &key, f);
    *filter = f;

    return INA_SUCCESS;
}

static void _prepare_headers(int num)
{
    int i;
    headers = (struct sockaddr_in*)ina_mem_alloc(sizeof(ina_net_udp_hdr_t)*num);
    for (i = 0; i < num; i++) {
        char *pch;
        const char *ip;
        int port;
        char *item = _strdup(ip_port[i]);
        pch = strtok(item, ":");
        ip = pch;
        pch = strtok(NULL, ":");
        port = atoi(pch);
        ina_mem_set(&headers[i], 0, sizeof(struct sockaddr_in));
        headers[i].sin_port = htons(port);
#ifdef WIN32
        InetPton(AF_INET, ip, &headers[i].sin_addr);
#else
        inet_aton(ip, &headers[i].sin_addr);
#endif
        free(item);
    }
}

static void _run_hash(int num_headers, size_t iterations)
{
    size_t i;
    for (i = 0; i < iterations*100000; i++) {
        __inafx_fast_filter_entry_t *f = NULL;
        int hi = rand() % num_headers;
        struct sockaddr_in *h = &headers[hi];
        INA_MUST_SUCCEED(__inafx_hash_tab_filter_apply(h, &f));
        if (f != NULL) {
            INA_MUST_SUCCEED(f->process_cb(h));
        }
        packet_counter++;
    }
}

static void _run_fast(int num_headers, size_t iterations)
{
    size_t i;
    for (i = 0; i < iterations*100000; i++) {
        __inafx_fast_filter_entry_t *f = NULL;
        int hi = rand() % num_headers;
        struct sockaddr_in *h = &headers[hi];
        INA_MUST_SUCCEED(__inafx_fast_eurex_filter_apply(h, &f));
        INA_MUST_SUCCEED(f->process_cb(h));
        packet_counter++;
    }
}

static void _cleanup_handler(int sig, int *error)
{
    if (stopwatch != NULL && INA_SUCCEED(ina_time_stopwatch_started(stopwatch))) {
        
        INA_TIME_STOPWATCH_STOP(stopwatch);

        printf("%s: Duration %f seconds\n", 
            ina_str_cstr(benchmark),
            stopwatch->tv->sec_duration);

        printf("Processed %u RDI packets\n", rdi_counter);
        printf("Processed %.2f packets per micro-second\n", 
            (packet_counter/stopwatch->tv->usec_duration));
        
        ina_time_stopwatch_destroy(&stopwatch);
    }
    if (__tab_filter != NULL) {
        __inafx_fast_filter_entry_t *f, *tf;
        HASH_ITER(hh, __tab_filter, f, tf) {
            HASH_DELETE(hh, __tab_filter, f);
            ina_mem_free(f);
        }
    }
    if (__mph_filter != NULL) {
        ina_mem_free(__mph_filter);
    }
    if (headers != NULL) {
        ina_mem_free(headers);
    }
    if (benchmark) {
        ina_str_free(benchmark);
    }
}

int main(int argc, char **argv)
{
    size_t iterations;
    int nipport = sizeof(ip_port)/(sizeof(char)*32);

    INA_OPTS(opt,
        INA_OPT_INT("i", "iterations", 1, "Number of million iterations"),
        INA_OPT_FLAG("n", "naive", "Execute the hashtable approach"),
        INA_OPT_FLAG("p", "mph", "Execute the approach with minimal perfect hash")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(_cleanup_handler);

    /* initialize random seed: */
    srand((unsigned int)time(NULL));
    
    _prepare_headers(nipport);

    if (!INA_SUCCEED(INA_TIME_STOPWATCH_CREATE(&stopwatch, 1, -1))) {
        return EXIT_FAILURE;
    }

    ina_opt_get_int("i", (int*)&iterations);
    
    if (INA_SUCCEED(ina_opt_isset("n"))) {
        __inafx_hash_tab_filter_create(&__tab_filter);
        __inafx_hash_tab_filter_add(__INAFX_FAST_EUREX_RDI_SNAPSHOT_P_ADDR, 
            __INAFX_FAST_EUREX_RDI_SNAPSHOT_P_PORT, 
            __inafx_fast_process_rdi_packet
        );
        __inafx_hash_tab_filter_add(__INAFX_FAST_EUREX_RDI_SNAPSHOT_S_ADDR, 
            __INAFX_FAST_EUREX_RDI_SNAPSHOT_S_PORT, 
            __inafx_fast_process_rdi_packet
        );
        __inafx_hash_tab_filter_add(__INAFX_FAST_EUREX_RDI_INCREMENTAL_P_ADDR, 
            __INAFX_FAST_EUREX_RDI_INCREMENTAL_P_PORT, 
            __inafx_fast_process_rdi_packet
        );
        __inafx_hash_tab_filter_add(__INAFX_FAST_EUREX_RDI_INCREMENTAL_S_ADDR, 
            __INAFX_FAST_EUREX_RDI_INCREMENTAL_S_PORT, 
            __inafx_fast_process_rdi_packet
        );
        benchmark = ina_str_new_fromcstr("hashtable");
       
        INA_TIME_STOPWATCH_START(stopwatch);
        _run_hash(nipport, iterations);
    }
    else if (INA_SUCCEED(ina_opt_isset("p"))) {
        __inafx_fast_eurex_filter_create(&__mph_filter);
        __inafx_fast_eurex_filter_add(__INAFX_FAST_EUREX_RDI_SNAPSHOT_P_ADDR, 
            __INAFX_FAST_EUREX_RDI_SNAPSHOT_P_PORT, 
            __inafx_fast_process_rdi_packet
        );
        __inafx_fast_eurex_filter_add(__INAFX_FAST_EUREX_RDI_SNAPSHOT_S_ADDR, 
            __INAFX_FAST_EUREX_RDI_SNAPSHOT_S_PORT, 
            __inafx_fast_process_rdi_packet
        );
        __inafx_fast_eurex_filter_add(__INAFX_FAST_EUREX_RDI_INCREMENTAL_P_ADDR, 
            __INAFX_FAST_EUREX_RDI_INCREMENTAL_P_PORT, 
            __inafx_fast_process_rdi_packet
        );
        __inafx_fast_eurex_filter_add(__INAFX_FAST_EUREX_RDI_INCREMENTAL_S_ADDR, 
            __INAFX_FAST_EUREX_RDI_INCREMENTAL_S_PORT, 
            __inafx_fast_process_rdi_packet
        );
        benchmark = ina_str_new_fromcstr("mph");
     
        INA_TIME_STOPWATCH_START(stopwatch);
        _run_fast(nipport, iterations);
    }
    else {
        printf("Invalid benchmark!\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

