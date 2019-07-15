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
    strncpy(buffer, "hello", 1023);
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
        strncpy(buffer, "hello", 1023);
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
    char *mac = (char*)malloc(sizeof(char)*6);
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
    INA_TEST_ASSERT_SUCCEED(ina_net_get_mac_addr(test_ip, mac, 6));

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
        INA_TEST_ASSERT_SUCCEED(ina_net_get_mac_addr(ip, mac, 6));
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
INA_TEST(net_local, system_lookup)
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

INA_TEST(net_local, resolve_host)
{
    ina_str_t ip = ina_str_new(128);
    INA_TEST_ASSERT_SUCCEED(ina_net_resolve("localhost", &ip));
    INA_TEST_MSG("IP for localhost: %s", ip);
    ina_str_free(ip);
}

INA_TEST(net_local, hostname)
{
    char host[128];
    INA_TEST_ASSERT_SUCCEED(ina_net_hostname(&host[0],127 ));
    INA_TEST_MSG("hostname: %s", host);
}

INA_TEST(net, invalid_arguments)
{
    short ac;
    ina_str_t *strptr = NULL;
    ina_str_t str = NULL;
    unsigned char b[1024];
    unsigned char *buf = &b[0];
    char chrb[1024];
    char *chrbuf = &chrb[0];
    ina_fd_t fd = 0;
    int port = 0;
    int nb_read;
    int nb_write;
    struct iovec io;
    struct msghdr msghdr;
    int nb_send = 0;
    ina_net_udp_receiver_t *receiver = NULL;
    int fake = 1;
    ina_net_pollfd_t pfd;
    nfds_t nfds = 0;
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_system_lookup(NULL, &ac, &strptr));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_system_lookup("localhost", NULL, &strptr));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_system_lookup("localhost", &ac, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_hostname(NULL, 128));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_tcp_server(NULL, 100, "127.0.0.1"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_tcp_server(&fd, -1, "127.0.0.1"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_tcp_server(&fd, INT_MAX, "127.0.0.1"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_tcp_server(&fd, 100, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_tcp_accept(NULL, fd, str, &port));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_tcp_accept(&fd, 0, str, &port));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_tcp_connect(NULL, "127.0.0.1", port, 0));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_tcp_connect(&fd, NULL, port, 0));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_tcp_connect(&fd, "127.0.0.1", -1, 0));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_tcp_connect(&fd, "127.0.0.1", INT_MAX, 0));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_nonblock(0));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_block(0));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_read(0, buf, 1024, &nb_read));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_read(1, NULL, 1024, &nb_read));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_read(1, buf, 0, &nb_read));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_read(1, buf, 1024, NULL));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_write(0, buf, 1024, &nb_write));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_write(1, NULL, 1024, &nb_write));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_write(1, buf, 0, &nb_write));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_write(1, buf, 1024, NULL));

    str = ina_str_new(31);
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_resolve(NULL, &str));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_resolve("127.0.0.1", NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_resolve("127.0.0.1", &str));
    ina_str_free(str);

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_readv(0, &io, 1, &nb_read));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_readv(1, NULL, 1, &nb_read));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_readv(1, &io, 0, &nb_read));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_readv(fd, &io, 1, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_writev(0, &io, 1, &nb_write));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_writev(1, NULL, 1, &nb_write));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_writev(1, &io, 0, &nb_write));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_writev(fd, &io, 1, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_sendmsg(0, &msghdr, 0, &nb_send));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_sendmsg(1, NULL, 0, &nb_send));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_sendmsg(1, &msghdr, 0, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_close(0));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_set_read_timeout(0, 100));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_set_read_timeout(1, 0));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_set_write_timeout(0, 100));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_set_write_timeout(1, 0));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_get_mac_addr("127.0.0.1", NULL, 1024));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_get_mac_addr("127.0.0.1", chrbuf, 3));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_bind(NULL, "127.0.0.1", 25));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_bind(&fd, NULL, 25));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_bind(&fd, "127.0.0.1", 0));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_bind(&fd, "127.0.0.1", INT_MAX));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,  ina_net_udp_socket(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT,  ina_net_udp_socket(&fd));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_receiver_new(NULL, 25, &receiver));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_receiver_new("127.0.0.1", 0, &receiver));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_receiver_new("127.0.0.1", INT_MAX, &receiver));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_receiver_new("127.0.0.1", 25, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_join_group(0, "127.0.0.1", "192.1.1.21"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_join_group(1, NULL, "192.1.1.21"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_join_group(1, "127.0.0.1", NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_leave_group(0, "127.0.0.1", "192.1.1.21"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_leave_group(1, NULL, "192.1.1.21"));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_leave_group(1, "127.0.0.1", NULL));

    receiver = (ina_net_udp_receiver_t*)&fake;
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_send(0, receiver, buf, 1024, &nb_write));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_send(1, NULL, buf, 1024, &nb_write));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_send(fd, receiver, NULL, 1024, &nb_write));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_send(fd, receiver, buf, 0, &nb_write));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_udp_send(fd, receiver, buf, 1024, NULL));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_poll(NULL, nfds, 0, &fake));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_net_poll(&pfd, nfds, 0, NULL));















}