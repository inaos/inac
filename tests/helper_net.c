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


INA_TEST_HELPER(net, non_blocking_echo_server) {

    int fd;
    int cfd;
    const char *addr = "127.0.0.1";
    int port = 8033;
    unsigned char buffer[100];
    int nb_read;

    if (!INA_SUCCEED(ina_net_tcp_server(&fd, port, addr))) {
        *retval = ina_err_peek();
        return;
     }

     if (!INA_SUCCEED(ina_net_nonblock(fd))) {
         ina_net_close(fd);
         *retval = ina_err_peek();
         return;
     }

     while (1) {
        if (INA_SUCCEED(ina_net_tcp_accept(&cfd, fd, NULL, NULL))) {
            if (cfd != -1) {
                if (!INA_SUCCEED(ina_net_nonblock(cfd))) {
                    ina_net_close(cfd);
                    cfd = -1;
                }
            }
        }

        if (cfd == -1) {
            if (INA_SUCCEED(ina_net_read(cfd, buffer, 100, &nb_read))) {
                if (nb_read > 0) {
                    ina_net_write(cfd, buffer, nb_read, &nb_read);
               }
           }
       }
       ina_time_sleep(300);
   }
}
