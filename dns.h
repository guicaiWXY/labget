//
// Created by admin on 2016/12/11.
//

#ifndef LABGET_DNS_H
#define LABGET_DNS_H
#include <string>
#include <strings.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <pthread.h>

#include "io.h"

#pragma comment(lib, "ws2_32.lib")

extern const unsigned int UDP_PORT;
extern const char* DNS_SERVER_ADDRESS;
extern const unsigned short DNS_PORT;


int* dns_look_up(std::string, int);
int* dns_look_up_v6(std::string, int);

int* dns_look_up(std::string, int, bool);
#endif //LABGET_DNS_H
