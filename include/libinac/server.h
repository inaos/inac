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
 
#ifndef _LIBINAC_SERVER_H_
#define _LIBINAC_SERVER_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DESIGN considerations:
 * + network server, designed to orchestrate an application-framework
 * + budget oriented
 * + Using poll to dispatch file and network I/O
 * + If high-throughput or low-latency is required we need a
 *   HW accelerated component that dispatches via shared-memory
 * + Use timer for time based tasks
 * + Everything should be mempool based
 * + Pull API to make it ljit friendly
 * + Actions are:
 *   - TCP connections
 *   - TCP read/write socket readiness
 *   - UDP read/write socket readiness
 *   - I/O read/write readiness
 *   - Timer/Task expiry
 * + Action should be dispatched without the need to branch,
 *   but also without the explicit usage of C function pointers.
 *   Although if the server is writting in C this does makes sense,
 *   if the server is written in Lua, the callbacks should rather be 
 *   Lua functions and therefore we need a flexible model.
 * + Examples for dispatching should be given in the tests C and Lua
 * 
 * Links:
 * + Good explanation of Level vs. Edge Triggered I/O
 *   https://medium.com/@copyconstruct/nonblocking-i-o-99948ad7c957  
 *
 */

/* opaque server context */
typedef struct ina_server_ctx_s ina_server_ctx_t;

typedef struct ina_server_descriptor_s {
    uint8_t timer_use_rdtsc;
    uint8_t hw_acceleration;
    uint8_t max_tcp_server_sockets;
    uint16_t max_tcp_client_connections;
    uint16_t max_udp_listeners;
    uint16_t max_udp_senders;
    uint16_t max_timer_tasks;
    uint32_t max_tcp_server_clients;
    ina_net_hw_backend_t hw_backend;
    size_t max_memory_utilization;
} ina_server_descriptor_t;

typedef struct ina_server_resource_requirements_s {
     size_t required_memory;
} ina_server_resource_requirements_t;

/* Actions: currently 255 seem more then enough */
typedef uint8_t ina_server_action_t;

#define INA_SERVER_ACTION_TCP_CONN_ACCEPT     0
#define INA_SERVER_ACTION_TCP_CONN_CLOSE      1
#define INA_SERVER_ACTION_TCP_READ            2
#define INA_SERVER_ACTION_TCP_WRITE           3

#define INA_SERVER_ACTION_UDP_SEND            4
#define INA_SERVER_ACTION_UDP_RECEIVE         5

#define INA_SERVER_ACTION_TIMER_EXPIRED       6

#define INA_SERVER_ACTION_FILE_OPEN           7
#define INA_SERVER_ACTION_FILE_CLOSE          8
#define INA_SERVER_ACTION_FILE_READ           9
#define INA_SERVER_ACTION_FILE_WRITE         10

/* opaque tcp connection */
typedef struct ina_server_tcp_connection_s ina_server_tcp_connection_t;

/* opaque tcp connection cursor */
typedef struct ina_server_tcp_connection_cursor_s ina_server_tcp_connection_cursor_t;

/*
 *
 */
INA_API(ina_rc_t) ina_server_init(ina_server_descriptor_t *desc,
                                  ina_server_ctx_t **ctx,
                                  ina_server_resource_requirements_t **req);

/*
 *
 */
INA_API(ina_rc_t) ina_server_destroy(ina_server_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_server_dispatch_action(ina_server_ctx_t *ctx, ina_server_action_t *action, void **data);

/*
 *
 */
INA_API(ina_rc_t) ina_server_register_tcp_server(ina_server_ctx_t *ctx, const char *eth, int port, 
                                                 uint32_t clients, int *fd, ina_server_resource_requirements_t **rq);

/*
 *
 */
INA_API(ina_rc_t) ina_server_register_tcp_client(ina_server_ctx_t *ctx, const char *eth, int port,
                                                 int *fd, ina_server_resource_requirements_t **rq);

/*
 *
 */
INA_API(ina_rc_t) ina_server_register_udp_listener(ina_server_ctx_t *ctx, const char *eth, int port,
                                                   int *fd, ina_server_resource_requirements_t **rq);

/*
 *
 */
INA_API(ina_rc_t) ina_server_register_udp_sender(ina_server_ctx_t *ctx, const char *address, int port,
                                                 int *fd, ina_server_resource_requirements_t **rq);

/*
 *
 */
INA_API(ina_rc_t) ina_server_register_task(ina_server_ctx_t *ctx, time_t millis, ina_time_event_t **event);

/*
 *
 */
INA_API(ina_rc_t) ina_server_tcp_connection_cursor_new(ina_server_ctx_t *ctx, ina_server_tcp_connection_cursor_t **cursor);

/*
 *
 */
INA_API(ina_rc_t) ina_server_tcp_connection_cursor_next(ina_server_tcp_connection_cursor_t *cursor, ina_server_tcp_connection_t **conn);

/*
 *
 */
INA_API(ina_rc_t) ina_server_tcp_connection_cursor_free(ina_server_ctx_t *ctx, ina_server_tcp_connection_cursor_t **cursor);

/*
 *
 */
INA_API(ina_rc_t) ina_server_tcp_connection_close(ina_server_ctx_t *ctx, ina_server_tcp_connection_t *conn);

/*
 *
 */
INA_API(ina_rc_t) ina_server_read(ina_server_ctx_t *ctx, int fd, size_t buffer_size, size_t *read, unsigned char *buffer);

/*
 *
 */
INA_API(ina_rc_t) ina_server_write(ina_server_ctx_t *ctx, int fd, size_t write, size_t *wrote, const unsigned char *buffer);

#ifdef __cplusplus
}
#endif

#endif

