/*
 * Copyright (c) 2013-2014, INAOS GmbH
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
#ifndef _LIBINAC_SSL_H_
#define _LIBINAC_SSL_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Accept self signed certificates (clients only) */
#define INA_SSL_ACCEPT_SELF_SIGNED (0x01)
/* Do blocking reads */  
#define INA_SSL_BLOCKING           (0x02)
/* Default options for server */
#define INA_SSL_SERVER_DEFAULT     (0x00)
/* Default option for client  */
#define INA_SSL_CLIENT_DEFAULT     INA_SSL_ACCEPT_SELF_SIGNED|INA_SSL_BLOCKING


/* Opaque structure representing a SSL context */
typedef struct ina_ssl_ctx_s ina_ssl_ctx_t;

/* Opaque structure representig a SSL connection */
typedef struct ina_ssl_cn_s  ina_ssl_cn_t;

/*
 * Initialize a SSL context. 
 *
 * Parameters:
 * ctx          Pointer to a context pointer to hold the newly created context.
 * num_sessions Allow number of sessions
 * options      Context options.
 *
 * Return:
 * INA_SUCCESS  if no error occured.
 * RC INA_EINIT if initializiation was failed.
 */
INA_API(ina_rc_t) ina_ssl_init(ina_ssl_ctx_t **ctx, 
                               int32_t num_sessions, 
                               uint32_t options);
/*
 * Destroy a SSL context.
 *
 * Parameters:
 * ctx      Context to destroy. ctx is NULL afer call.
 *
 * Return:
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ssl_destroy(ina_ssl_ctx_t **ctx);

/*
 * Initialize a new SSL client-side connection.
 *
 * Paramaters:
 * ctx      SSL context.
 * cn       Pointer to a connection pointer to hold the newly created client-
 *          side connection.
 * fd       Socket file descriptor.
 *
 * Return:
 * INA_SUCCESS if no error occurred.
 */
INA_API(ina_rc_t) ina_ssl_client_new(ina_ssl_ctx_t *ctx, 
                                     ina_ssl_cn_t **cn, 
                                     int fd);
/*
 * Free a SSL client-side connection.
 *
 * Parameters:
 * ctx      SSL context.
 * cn       SSL client connection to free. cn is NULL after on return.
 *
 * Return:
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ssl_client_free(ina_ssl_ctx_t *ctx, ina_ssl_cn_t **cn);

/*
 * Accepet a new SSL server-side connection.
 * 
 * Parameters:
 * ctx          SSL context.
 * cn           Pointer to an SSL server-side connection pointer to hold the 
 *              newly create server-side connection.
 * client_fd    Client socket file descriptor.
 * 
 * Return:
 * INA_SUCCES if no error occurred.
 */
INA_API(ina_rc_t) ina_ssl_server_new(ina_ssl_ctx_t *ctx, 
                                     ina_ssl_cn_t **cn, 
                                     int client_fd);
/*
 * Free a SSL server-side connection.
 *
 * Parameters:
 * ctx      SSL context.
 * cn       SSL server-side connection to free. cn is NULL after on return.
 *
 * Return:
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ssl_server_free(ina_ssl_ctx_t *ctx, ina_ssl_cn_t **cn);

/*
 * Read SSL encrypted data.
 *
 * Parameters:
 * cn           SSL connection.
 * buf          Pointer to a buffer.
 * bytes_read   Contains number of bytes read on return.
 * 
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_EAGAIN on non blocking read.
 */
INA_API(ina_rc_t) ina_ssl_read(ina_ssl_cn_t *cn, 
                               unsigned char **buf, 
                               size_t *bytes_read);
/*
 * Write data with SSL encryption.
 * 
 * Parameters:
 * cn           SSL connection.
 * buf          Pointer to a buffer.
 * buf_len      Buffer size in bytes.
 * bytes_read   Contains number of written on return
 * 
 * Return:
 * INA_SUCCESS if no error occured.
 */
INA_API(ina_rc_t) ina_ssl_write(ina_ssl_cn_t *cn,
                                const unsigned char *buf, 
                                size_t buf_len, 
                                size_t *bytes_written);
/*
 * Force the client to perform its handshake again.For a client this involves 
 * sending another "client hello" message. For the server is means sending 
 * a "hello request" message. This is a blocking call on the client 
 * (until the handshake completes).
 *
 * Parameters:
 * cn       SSL connection
 *
 * Return:
 * INA_SUCCESS if the handshake is complete and ok.
 */
INA_API(ina_rc_t) ina_ssl_handshake(ina_ssl_cn_t *cn);

/*
 * Query the status of the handshake.
 *
 * Parameters:
 * cn       SSL connection
 *
 * Return:
 * INA_SUCCESS if the handshake is complete and ok.
 * RC INA_EGAIN if handshake is still in progress.
 */
INA_API(ina_rc_t) ina_ssl_handshake_status(ina_ssl_cn_t *cn);
			
#ifdef __cplusplus
}
#endif

#endif
