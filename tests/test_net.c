/*
 * Copyright (c) 2013-2015, INAOS GmbH
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

#ifdef INA_OS_WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#endif

#include <libinac/lib.h>

#ifdef INA_OS_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netdb.h>
#endif

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
    INA_TEST_HELPER_TERMINATE(&data->hid);
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
#ifdef INA_OS_WIN32
INA_TEST(net, mac_addr)
{
    char *mac = (char*)malloc(sizeof(6));
    char *test_ip;
    int found = 0;

    /* first the get first IP-Address of the system */
#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    unsigned int i = 0;

    ULONG family = AF_INET;
    ULONG flags = GAA_FLAG_SKIP_DNS_SERVER;
    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
    PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
    IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
    IP_ADAPTER_PREFIX *pPrefix = NULL;

    outBufLen = WORKING_BUFFER_SIZE;
    do {
        pAddresses = (IP_ADAPTER_ADDRESSES *)ina_mem_alloc(outBufLen);
        INA_TEST_ASSERT_NOT_NULL(pAddresses);
        dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);
        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            ina_mem_free(pAddresses);
            pAddresses = NULL;
        }
        else {
            break;
        }
        Iterations++;
    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

    if (dwRetVal == NO_ERROR) {
        pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            pUnicast = pCurrAddresses->FirstUnicastAddress;
            if (pUnicast != NULL) {
                for (i = 0; pUnicast != NULL; i++) {
                    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                        struct sockaddr_in *sin = (struct sockaddr_in*)pUnicast->Address.lpSockaddr;
                        char *ip = inet_ntoa(sin->sin_addr);
                        if (!found && strncmp("127.", ip, 4) != 0) {
                            test_ip = _strdup(ip);
                            found = 1;
                        }
                    }
                    pUnicast = pUnicast->Next;
                }
            }
            pCurrAddresses = pCurrAddresses->Next;
        }
    }
    if (pAddresses) {
        ina_mem_free(pAddresses);
    }

    /* execute the actual test now that we have an IP address */
    INA_TEST_ASSERT_SUCCEED(ina_net_get_mac_addr(test_ip, mac));

    free(test_ip);
}
#else
INA_TEST(net, mac_addr)
{
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    char *ip = NULL;
    char *mac = (char*)malloc(sizeof(6));

    INA_TEST_ASSERT_FALSE(getifaddrs(&ifaddr) == -1);

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }
        getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (strcmp(ifa->ifa_name, "eth0") == 0) {
            ip = strdup(host);
            break;
        }
    }

    INA_TEST_ASSERT_SUCCEED(ina_net_get_mac_addr(ip, mac));
    
    free(ip);
    freeifaddrs(ifaddr);
}
#endif
