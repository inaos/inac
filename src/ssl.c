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

static void __ina_ssl_error_lookup(int err, char *msg)
{
    switch (err) {
        case SSL_ERROR_DEAD:
            strcpy(msg, "SSL_ERROR_DEAD");
            break;
        case SSL_CLOSE_NOTIFY:
            strcpy(msg, "SSL_ERROR_CLOSE_NOTIFY");
            break;
        case SSL_ERROR_CONN_LOST:
            strcpy(msg, "SSL_ERROR_CONN_LOST");
            break;
        case SSL_ERROR_SOCK_SETUP_FAILURE:
            strcpy(msg, "SSL_ERROR_SOCK_SETUP_FAILURE");
            break;
        case SSL_ERROR_INVALID_HANDSHAKE:
            strcpy(msg, "SSL_ERROR_INVALID_HANDSHAKE");
            break;
        case SSL_ERROR_INVALID_PROT_MSG:
            strcpy(msg, "SSL_ERROR_INVALID_PROT_MSG");
            break;
        case SSL_ERROR_INVALID_HMAC:
            strcpy(msg, "SSL_ERROR_INVALID_HMAC");
            break;
        case SSL_ERROR_INVALID_VERSION:
            strcpy(msg, "SSL_ERROR_INVALID_VERSION");
            break;
        case SSL_ERROR_INVALID_SESSION:
            strcpy(msg, "SSL_ERROR_INVALID_SESSION");
            break;
        case SSL_ERROR_NO_CIPHER:
            strcpy(msg, "SSL_ERROR_NO_CIPHER");
            break;
        case SSL_ERROR_BAD_CERTIFICATE:
            strcpy(msg, "SSL_ERROR_BAD_CERTIFICATE");
            break;
        case SSL_ERROR_INVALID_KEY:
            strcpy(msg, "SSL_ERROR_INVALID_KEY");
            break;
        case SSL_ERROR_FINISHED_INVALID:
            strcpy(msg, "SSL_ERROR_FINISHED_INVALID");
            break;
        case SSL_ERROR_NO_CERT_DEFINED:
            strcpy(msg, "SSL_ERROR_NO_CERT_DEFINED");
            break;
        case SSL_ERROR_NO_CLIENT_RENOG:
            strcpy(msg, "SSL_ERROR_NO_CLIENT_RENOG");
            break;
        case SSL_ERROR_NOT_SUPPORTED:
            strcpy(msg, "SSL_ERROR_NOT_SUPPORTED");
            break;
        default:
            strcpy(msg, "SSL_UNKNOWN_ERROR");
            break; 
    }
}

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

INA_API(ina_rc_t) ina_ssl_read(ina_ssl_conn_t *conn, unsigned char **buf, size_t *bytes_read)
{
    int ret;

    INA_ASSERT_NOTNULL(conn);
    ret = ssl_read(conn->conn, buf);
    if (ret == SSL_OK) {
        return INA_SSL_EAGAIN;    
    }
    if (ret < 0) {
        char err[128];
        __ina_ssl_error_lookup(ret, err);
        return INA_SSL_ERROR(INA_EREAD, err);    
    }
    *bytes_read = ret;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_write(ina_ssl_conn_t *conn, unsigned char *buf, size_t buf_len, 
                                size_t *bytes_written)
{
    int ret;
    
    INA_ASSERT_NOTNULL(conn);
    ret = ssl_write(conn->conn, buf, buf_len);
    if (ret < 0) {
        char err[128];
        __ina_ssl_error_lookup(ret, err);
        return INA_SSL_ERROR(INA_EREAD, err);
    }
    *bytes_written = ret;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_handshake_status(ina_ssl_conn_t *conn)
{
    int ret;
    
    INA_ASSERT_NOTNULL(conn);
    ret = ssl_handshake_status(conn->conn);
    if (ret != SSL_OK) {
        return INA_SSL_EAGAIN;
    }

    return INA_SUCCESS;
}

