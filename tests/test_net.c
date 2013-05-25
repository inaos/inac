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

INA_TEST_DATA(net) {
    ina_test_hid_t hid;
    int server_fd;
    int client_fd;
    int fd;
};

INA_TEST_SETUP(net) {
    INA_TEST_HELPER_INVOKE(&data->hid, net, non_blocking_echo_server, 
        __INA_TCP_ADDR, 
         INA_NUM2STR(__INA_TCP_PORT),
         NULL);
}

INA_TEST_TEARDOWN(net) {
    if (data->client_fd > -1) {
        ina_net_close(data->client_fd);
    }
    data->client_fd = -1;
    INA_TEST_HELPER_STOP(&data->hid);
}

INA_TEST_FIXTURE(net, tcp_connect_no_timeout) {
    INA_TEST_ASSERT_SUCCEED(ina_net_tcp_connect(&data->client_fd,  
                            __INA_TCP_ADDR, 
                            __INA_TCP_PORT,
                            0));
    INA_TEST_ASSERT_SUCCEED(ina_net_close(data->client_fd));
}

INA_TEST_FIXTURE(net, tcp_connect_5sec_timeout) {
    INA_TEST_ASSERT_SUCCEED(ina_net_tcp_connect(&data->client_fd,  
                            __INA_TCP_ADDR, 
                            __INA_TCP_PORT,
                            5000));
    INA_TEST_ASSERT_SUCCEED(ina_net_close(data->client_fd));
}

INA_TEST_FIXTURE(net, tcp_write_read) {
    char buffer[1024];
    int nb_read = 0;
    int nb_write = 0;
    
    INA_TEST_ASSERT_SUCCEED(ina_net_tcp_connect(&data->client_fd,  
                            __INA_TCP_ADDR, 
                            __INA_TCP_PORT,
                            5000));
    INA_TEST_MSG("conected to %s:%d", __INA_TCP_ADDR, __INA_TCP_PORT);

    ina_mem_set(buffer, 0, 1024);
    strcpy(buffer, "hello");
    INA_TEST_MSG("write %s", buffer);    
    INA_TEST_ASSERT_SUCCEED(ina_net_write(data->client_fd, 
                            (const unsigned char*)buffer,
                            strlen(buffer), &nb_write));

    ina_mem_set(buffer, 0, 1024);
    INA_TEST_ASSERT_SUCCEED(ina_net_read(data->client_fd, 
                            (unsigned char*)buffer, 1024,
                            &nb_read));
    INA_TEST_MSG("read %d bytes:%s", nb_read, buffer);
    INA_TEST_ASSERT_EQUAL_INTEGER(nb_read, nb_write);
}


INA_TEST_FIXTURE(net, tcp_write_read_1000_times) {
    char buffer[1024];
    int nb_read = 0;
    int nb_write = 0;
    int c = 100;
    
    INA_TEST_ASSERT_SUCCEED(ina_net_tcp_connect(&data->client_fd,  
                            __INA_TCP_ADDR, 
                            __INA_TCP_PORT,
                            5000));
    INA_TEST_MSG("conected to %s:%d", __INA_TCP_ADDR, __INA_TCP_PORT);
    
    INA_TEST_MSG("write/reed 100 times %s", buffer);
    while (c--) {
        ina_mem_set(buffer, 0, 1024);
        strcpy(buffer, "hello");
        INA_TEST_ASSERT_SUCCEED(ina_net_write(data->client_fd, 
                                (const unsigned char*)buffer,
                                strlen(buffer), &nb_write));

        ina_mem_set(buffer, 0, 1024);
        INA_TEST_ASSERT_SUCCEED(ina_net_read(data->client_fd, 
                                (unsigned char*)buffer, 1024,
                                &nb_read));
        INA_TEST_ASSERT_EQUAL_INTEGER(nb_read, nb_write);
    }
}

