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

#define __INA_TCP_ADDR "127.0.0.1"
#define __INA_TCP_PORT  8033

INA_TEST_DATA(ssl) {
    ina_test_hid_t hid;
    int client_fd;
};

INA_TEST_SETUP(ssl) {
    INA_TEST_HELPER_INVOKE(&data->hid, ssl, ssl_server,
        __INA_TCP_ADDR,
         INA_NUM2STR(__INA_TCP_PORT),
         NULL);
}

INA_TEST_TEARDOWN(ssl) {
    if (data->client_fd > -1) {
        ina_net_close(data->client_fd);
    }
    data->client_fd = -1;
    INA_TEST_HELPER_TERMINATE(&data->hid);
}

INA_TEST_FIXTURE(ssl, ssl_write_read) {
    unsigned char buffer[1024];
    unsigned char *readbuf = NULL;
    size_t nb_read = 0;
    size_t nb_write = 0;
    ina_ssl_ctx_t *ssl_ctx = NULL;
    ina_ssl_conn_t *ssl_conn = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_ssl_init(&ssl_ctx, 1));

    INA_TEST_ASSERT_SUCCEED(ina_net_tcp_connect(&data->client_fd,
                            __INA_TCP_ADDR,
                            __INA_TCP_PORT,
                            5000));
    INA_TEST_ASSERT_SUCCEED(ina_ssl_client_new(ssl_ctx, &ssl_conn, data->client_fd));

    INA_TEST_MSG("conected to %s:%d", __INA_TCP_ADDR, __INA_TCP_PORT);

    ina_mem_set(buffer, 0, 1024);
    strcpy((char*)buffer, "hello");
    INA_TEST_MSG("write %s", buffer);
    INA_TEST_ASSERT_SUCCEED(ina_ssl_write(ssl_conn,
                            buffer,
                            strlen((const char*)buffer), &nb_write));

    INA_TEST_ASSERT_SUCCEED(ina_ssl_read(ssl_conn,
                            &readbuf,
                            &nb_read));
    INA_TEST_MSG("read %d bytes:%s", nb_read, (const char*)readbuf);
    INA_TEST_ASSERT_EQUAL_INTEGER(nb_read, nb_write);

    INA_TEST_ASSERT_SUCCEED(ina_ssl_client_free(ssl_ctx, &ssl_conn));
    INA_TEST_ASSERT_SUCCEED(ina_ssl_destroy(&ssl_ctx));
}    
                            
