/*
 * Copyright (c) 2013, INAOS GmbH
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

#include <contribs/axtls/ssl.h>

struct ina_ssl_conn_s {
    int server;
    SSL *conn;
};

struct ina_ssl_ctx_s {
    uint32_t options;
    SSL_CTX *ssl;
    ina_ssl_conn_t *conn;
};

INA_API(ina_rc_t) ina_ssl_init(ina_ssl_ctx_t **ctx, int num_sessions)
{
    *ctx = (ina_ssl_ctx_t*)ina_mem_alloc(sizeof(struct ina_ssl_ctx_s));
    INA_ASSERT_NOTNULL(*ctx);
    
    (*ctx)->ssl = ssl_ctx_new((*ctx)->options, num_sessions);
    (*ctx)->conn = NULL;

    if ((*ctx)->ssl == NULL) {
        return INA_SSL_EINIT;
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_destroy(ina_ssl_ctx_t **ctx)
{
    ssl_ctx_free((*ctx)->ssl);
    ina_mem_free(*ctx);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_client_new(ina_ssl_ctx_t *ctx, ina_ssl_conn_t **conn, int fd)
{
    INA_ASSERT_NULL(ctx->conn);
    ctx->conn = (ina_ssl_conn_t*)ina_mem_alloc(sizeof(struct ina_ssl_conn_s));
    INA_ASSERT_NOTNULL(ctx->conn);

    ctx->conn->conn = ssl_client_new(ctx->ssl, fd, NULL, 0);
    if (ctx->conn->conn == NULL) {
        return INA_SSL_EINIT;
    }
    ctx->conn->server = 0;

    *conn = ctx->conn;
    
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_client_free(ina_ssl_ctx_t *ctx, ina_ssl_conn_t **conn)
{
    INA_ASSERT_NOTNULL(ctx->conn);
    INA_ASSERT_FALSE((*conn)->server);

    ssl_free(ctx->conn->conn);
    ina_mem_free(ctx->conn);
    ctx->conn = NULL;

    *conn = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_server_new(ina_ssl_ctx_t *ctx, ina_ssl_conn_t **conn, int client_fd)
{
    INA_ASSERT_NULL(ctx->conn);
    ctx->conn = (ina_ssl_conn_t*)ina_mem_alloc(sizeof(struct ina_ssl_conn_s));
    INA_ASSERT_NOTNULL(ctx->conn);

    ctx->conn->conn = ssl_server_new(ctx->ssl, client_fd);
    if (ctx->conn->conn == NULL) {
        return INA_SSL_EINIT;
    }
    ctx->conn->server = 1;

    *conn = ctx->conn;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_server_free(ina_ssl_ctx_t *ctx, ina_ssl_conn_t **conn)
{
    INA_ASSERT_NOTNULL(ctx->conn);
    INA_ASSERT_FALSE((*conn)->server);

    ssl_free(ctx->conn->conn);
    ina_mem_free(ctx->conn);
    ctx->conn = NULL;

    *conn = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_read(ina_ssl_conn_t *conn, unsigned char **buf, int *bytes_read)
{
    int ret;

    INA_ASSERT_NOTNULL(conn);
    ret = ssl_read(conn->conn, buf);
    if (ret == SSL_OK) {

    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_write(ina_ssl_conn_t *conn, unsigned char *buf, size_t buf_len)
{
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_handshake_status(ina_ssl_conn_t *conn)
{
    return INA_SUCCESS;
}
