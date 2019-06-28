/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

#ifndef INA_OS_WINDOWS
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

/*
 * Poor People Echo Server
 */
INA_TEST_HELPER(net, non_blocking_echo_server) {
    ina_fd_t fd = INA_NET_INVALID_SOCKET;
    ina_fd_t cfd = INA_NET_INVALID_SOCKET;
    const char *addr;
    int port;
    unsigned char buffer[4096];
    int nb_read;

    INA_TEST_HELPER_CHECK_ARGC(2);
    addr = INA_TEST_HELPER_CARG(0);
    port = INA_TEST_HELPER_IARG(1);
 
    ina_mem_set(buffer, 0, 4096);

    if (INA_FAILED(ina_net_tcp_server(&fd, port, addr))) {
        *retval = INA_RC_ERROR(ina_err_get_rc());
        return;
     }

     if (INA_FAILED(ina_net_nonblock(fd))) {
         ina_net_close(fd);
         *retval = INA_RC_ERROR(ina_err_get_rc());
         return;
     }

     while (1) {
        if (cfd == INA_NET_INVALID_SOCKET) {
            if (INA_SUCCEED(ina_net_tcp_accept(&cfd, fd, NULL, NULL))) {
                if (cfd != INA_NET_INVALID_SOCKET) {
                    if (INA_FAILED(ina_net_nonblock(cfd))) {
                        ina_net_close(cfd);
                        cfd = INA_NET_INVALID_SOCKET;
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
               cfd = INA_NET_INVALID_SOCKET;
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
	ina_fd_t s;
    struct sockaddr_in si_other;
    int i, slen=sizeof(si_other);
 
    INA_TEST_HELPER_CHECK_ARGC(2);
    addr = INA_TEST_HELPER_CARG(0);
    port = INA_TEST_HELPER_IARG(1);

	INA_TEST_ASSERT(port > 0);
	INA_TEST_ASSERT(port < UINT16_MAX);
 
    ina_mem_set(buf, 0, 512);
   
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <  0) {
        INA_TEST_HELPER_SET_RC(INA_ERR_FAILED);
        return;
    }        
    
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons((uint16_t)port);
    if (inet_addr(addr) != 0) {
        i = 0;
        while (1) {
            snprintf(buf, 511, "This is packet %d\n", ++i);
            if (sendto(s, buf, 512, 0, (struct sockaddr*)&si_other, slen) == -1) {
                INA_TEST_HELPER_SET_RC(INA_ERR_FAILED);
            }
        }
    }

#if INA_OS_WINDOWS
    closesocket(s);
#else
    close(s);
#endif
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
 }

