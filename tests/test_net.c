/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifdef INA_OS_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#endif

#include <libinac/lib.h>

#ifndef INA_OS_WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netdb.h>
#endif

#define __INA_TCP_ADDR "127.0.0.1"
#define __INA_TCP_PORT  8033

INA_TEST_DATA(net) {
    ina_test_hid_t hid;
    ina_fd_t server_fd;
    ina_fd_t client_fd;
    ina_fd_t fd;
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
    data->client_fd = INA_NET_INVALID_SOCKET;
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

INA_TEST_FIXTURE_SKIP(net, tcp_write_read) {
    char buffer[1024];
    int nb_read = 0;
    int nb_write = 0;
    
    INA_TEST_ASSERT_SUCCEED(ina_net_tcp_connect(&data->client_fd,  
                            __INA_TCP_ADDR, 
                            __INA_TCP_PORT,
                            5000));
    INA_TEST_MSG("conected to %s:%d", __INA_TCP_ADDR, __INA_TCP_PORT);

    ina_mem_set(buffer, 0, 1024);
    strncpy(buffer, 1023, "hello");
    INA_TEST_MSG("write %s", buffer);    
    INA_TEST_ASSERT_SUCCEED(ina_net_write(data->client_fd, 
                            (const unsigned char*)buffer,
                            (int)strlen(buffer), &nb_write));

    ina_mem_set(buffer, 0, 1024);
    INA_TEST_ASSERT_SUCCEED(ina_net_read(data->client_fd, 
                            (unsigned char*)buffer, 1024,
                            &nb_read));
    INA_TEST_MSG("read %d bytes:%s", nb_read, buffer);
    INA_TEST_ASSERT_EQUAL_INT(nb_read, nb_write);
}


INA_TEST_FIXTURE_SKIP(net, tcp_write_read_1000_times) {
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
        strncpy(buffer, 1023, "hello");
        INA_TEST_ASSERT_SUCCEED(ina_net_write(data->client_fd, 
                                (const unsigned char*)buffer,
                                (int)strlen(buffer), &nb_write));

        ina_mem_set(buffer, 0, 1024);
        INA_TEST_ASSERT_SUCCEED(ina_net_read(data->client_fd, 
                                (unsigned char*)buffer, 1024,
                                &nb_read));
        INA_TEST_ASSERT_EQUAL_INT(nb_read, nb_write);
    }
}
#ifdef INA_OS_WIN32
INA_TEST(net_local, mac_addr)
{
    char *mac = (char*)malloc(sizeof(6));
    char *test_ip = NULL;
    int found = 0;

    /* first the get first IP-Address of the system */
#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
    DWORD dwRetVal = 0;
    unsigned int i = 0;

    ULONG family = AF_INET;
    ULONG flags = GAA_FLAG_SKIP_DNS_SERVER;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    
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
						char ipbuf[128];
						struct sockaddr_in *sin = (struct sockaddr_in*)pUnicast->Address.lpSockaddr;
						inet_ntop(AF_INET, &sin->sin_addr, ipbuf, 128);
                        if (!found && strncmp("127.", ipbuf, 4) != 0) {
                            test_ip = _strdup(ipbuf);
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

    if (test_ip != NULL) {
        free(test_ip);
    }
    free(mac);
}
#else
INA_TEST(net_local, mac_addr)
{
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    char *ip = NULL;
    char *mac = (char*)malloc(6);
    int found = INA_NO;

    INA_TEST_ASSERT_FALSE(getifaddrs(&ifaddr) == -1);

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }
        getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (strcmp(ifa->ifa_name, "eth0") == 0 || strcmp(ifa->ifa_name, "en0") == 0) {
            ip = strdup(host);
            found = INA_YES;
            break;
        }
    }

    if (found) {
        INA_TEST_ASSERT_SUCCEED(ina_net_get_mac_addr(ip, mac));
        INA_TEST_MSG("MAC address for %s is %02X:%02X:%02X:%02X:%02X:%02X", ip,
                     mac[0],
                     mac[1],
                     mac[2],
                     mac[3],
                     mac[4],
                     mac[5]);
    } else {
        INA_TEST_MSG("%s", "No interface eth0 found!");
    }

    if (ip != NULL) {
        free(ip);
    }
    free(mac);
    freeifaddrs(ifaddr);
}
#endif
INA_TEST_SKIP(net_local, system_lookup)
{
    ina_str_t *addresses;
    short      address_count;

    INA_TEST_ASSERT_SUCCEED(ina_net_system_lookup("localhost", &address_count, &addresses));
    INA_TEST_ASSERT_TRUE(address_count > 0);
    for (int n = 0; n < address_count; ++n) {
       INA_TEST_MSG("address %d: %s", n, addresses[n]);
       ina_str_free(addresses[n]);
    }
    INA_TEST_ASSERT_FAILED(ina_net_system_lookup("blablabla", &address_count, &addresses));
}
