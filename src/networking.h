#ifndef __TEMPESP_NETWORKING_H__
#define __TEMPESP_NETWORKING_H__

#include <stdbool.h>

struct report_t;

bool init_udp_socket();
bool init_server_addr(const char* hostname, uint16_t port);
bool send_report(struct report_t* report);
bool receive_server_response();

#endif