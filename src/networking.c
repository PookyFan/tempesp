#include <string.h>
#include "lwip/netdb.h"
//#include "lwip/api.h"

#include "data.h"
#include "networking.h"

#define NETWORK_TIMEOUT 1 //in seconds

static int udp_socket;
static struct sockaddr* server_addr;

bool init_udp_socket()
{
    udp_socket = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket < 0)
    {
        printf("Could not create UDP socket\n");
        return false;
    }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family      = AF_INET;
    local_addr.sin_port        = PP_HTONS(INADDR_ANY);
    local_addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
    if( lwip_bind(udp_socket, (struct sockaddr*)&local_addr, sizeof(local_addr)) )
    {
        printf("Could not bind UDP socket\n");
        return false;
    }

    const struct timeval timeout = { NETWORK_TIMEOUT, 0 };
    if( setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) )
    {
        printf("Could not set ops timeout on UDP socket\n");
        return -1;
    }

    return true;
}

bool init_server_addr(const char* hostname, uint16_t port)
{
    const struct addrinfo hints = { .ai_family = AF_UNSPEC, .ai_socktype = SOCK_DGRAM };
    struct addrinfo* lookup_addr;
    int err = getaddrinfo(hostname, NULL, &hints, &lookup_addr);
    if(err != 0 || lookup_addr == NULL)
    {
        printf("DNS lookup failed - err=%d lookup_addr=%p\n", err, lookup_addr);
        return false;
    }

    server_addr = lookup_addr->ai_addr;
    if(server_addr->sa_family != AF_INET)
    {
        printf("DNS lookup did not yield proper IP address\n");
        freeaddrinfo(lookup_addr);
        return false;
    }

    ((struct sockaddr_in*)server_addr)->sin_port = PP_HTONS(port);
    printf("DNS lookup for %s succeeded\nServer IP=%s port=%d\n",
           hostname, inet_ntoa( ((struct sockaddr_in*)server_addr)->sin_addr ), port);
    return true;
}

bool send_report(struct report_t* report)
{
    const int size_to_send = sizeof(*report);
    uint8_t* ptr = (uint8_t*)report;
    for(int size_sent = 0; size_sent < size_to_send;)
    {
        int result = lwip_sendto(
            udp_socket, ptr + size_sent, size_to_send - size_sent, 0, server_addr, sizeof(*server_addr));
        if(result <= 0)
        {
            printf("Could not send report to server (%d of %d bytes sent) - err:%d\n",
                   size_sent, size_to_send, result);
            return false;
        }
        size_sent += result;
    }
    return true;
}

bool receive_server_response()
{
    uint8_t data[4] = {0};
    int result  = lwip_recvfrom(udp_socket, data, sizeof(data), 0, server_addr, NULL);
    printf("Receive data status: %d, data: %x %x %x %x\n", result, data[0], data[1], data[2], data[3]);
    return result == sizeof(data);
}
