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

static ina_ssl_ctx_t *__ssl = NULL;

static void __cleanup_handler(int sig, int *error)
{
    ina_ssl_destroy(&__ssl);
    *error = EXIT_SUCCESS;
}

INA_TEST_HELPER(ssl, ssl_server) {
    const char *addr;
    int32_t port;
    ina_ssl_cn_t *ssl_cn;
    int fd = -1;
    int cfd = -1;
    unsigned char *buffer;
    size_t nb_read;
    
    INA_TEST_HELPER_CHECK_ARGC(2);
    addr = INA_TEST_HELPER_CARG(0);
    port = INA_TEST_HELPER_IARG(1);
    
    ina_set_cleanup_handler(__cleanup_handler);
    
    if (!INA_SUCCEED(ina_ssl_init(&__ssl, 1, 0))) {
        INA_TEST_HELPER_SET_RC(ina_err_peek());
        return;
    }

    if (!INA_SUCCEED(ina_net_tcp_server(&fd, port, addr))) {
        INA_TEST_HELPER_SET_RC(ina_err_peek());
        return;
    }

    if (!INA_SUCCEED(ina_net_nonblock(fd))) {
        ina_net_close(fd);
        INA_TEST_HELPER_SET_RC(ina_err_peek());
        return;
    }

    while (1) {
        if (cfd == -1) {
            if (INA_SUCCEED(ina_net_tcp_accept(&cfd, fd, NULL, NULL))) {
                if (cfd != -1) {
                    if (!INA_SUCCEED(ina_net_nonblock(cfd))) {
                        ina_net_close(cfd);
                        cfd = -1;
                    }
                    if (!INA_SUCCEED(ina_ssl_server_new(__ssl, &ssl_cn, cfd))) {
                        INA_TEST_HELPER_SET_RC(ina_err_peek());
                        return;
                    }
                }
            }
        }

        if (cfd != -1) {
            
            ina_rc_t rc = ina_ssl_read(ssl_cn, &buffer, &nb_read);

            if (INA_RC_REASON(rc) == INA_EAGAIN) {
                if (ina_ssl_handshake_status(ssl_cn) != INA_SUCCESS) {
                    continue;
                }
            } else if (rc == INA_SUCCESS) {
                if (nb_read > 0) {
                    ina_ssl_write(ssl_cn, buffer, nb_read, &nb_read);
                }
            } else {
               ina_ssl_server_free(__ssl, &ssl_cn);
               ina_net_close(cfd);
               cfd = -1;
            }
       }
       ina_time_sleep(300);
    }

    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
}
