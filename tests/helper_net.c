/*
 * Copyright (c) 2013-2018, INAOS GmbH
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

#ifndef INA_OS_WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

/*
 * Poor People Echo Server
 */
INA_TEST_HELPER(net, non_blocking_echo_server) {

    int fd = -1;
    int cfd = -1;
    const char *addr;
    int port;
    unsigned char buffer[4096];
    int nb_read;

    INA_TEST_HELPER_CHECK_ARGC(2);
    addr = INA_TEST_HELPER_CARG(0);
    port = INA_TEST_HELPER_IARG(1);
 
    ina_mem_set(buffer, 0, 4096);

    if (INA_FAILED(ina_net_tcp_server(&fd, port, addr))) {
        *retval = ina_err_get_last_rc();
        return;
     }

     if (INA_FAILED(ina_net_nonblock(fd))) {
         ina_net_close(fd);
         *retval = ina_err_get_last_rc();
         return;
     }

     while (1) {
        if (cfd == -1) {
            if (INA_SUCCEED(ina_net_tcp_accept(&cfd, fd, NULL, NULL))) {
                if (cfd != -1) {
                    if (INA_FAILED(ina_net_nonblock(cfd))) {
                        ina_net_close(cfd);
                        cfd = -1;
                    }
                }
            }
        }

        if (cfd != -1) {
            if (INA_SUCCEED(ina_net_read(cfd, buffer, 4096, &nb_read))) {
                if (nb_read > 0) {
                    ina_net_write(cfd, buffer, nb_read, &nb_read);
               }
           } else {
               ina_net_close(cfd);
               cfd = -1;
            }
       }
       ina_time_sleep(300);
   }
}


/*
 * Poor People UDP sender
 */
INA_TEST_HELPER(net, udp_sender) {

    const char *addr;
    int port;

    char buf[512];
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
 
    INA_TEST_HELPER_CHECK_ARGC(2);
    addr = INA_TEST_HELPER_CARG(0);
    port = INA_TEST_HELPER_IARG(1);
 
    ina_mem_set(buf, 0, 512);
   
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <  0) {
        INA_TEST_HELPER_SET_RC(INA_ERR_FAILED);
        return;
    }        
    
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(port);
    if (inet_addr(addr) != 0) {
        i = 0;
        while (1) {
            sprintf(buf, "This is packet %d\n", ++i);
            if (sendto(s, buf, 512, 0, (struct sockaddr*)&si_other, slen) == -1) {
                INA_TEST_HELPER_SET_RC(INA_ERR_FAILED);
            }
        }
    }

#if INA_OS_WIN32
    closesocket(s);
#else
    close(s);
#endif
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
 }

