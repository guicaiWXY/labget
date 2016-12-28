//
// Created by wxy on 12/14/16.
//

#ifndef LABGET_HTTP_H
#define LABGET_HTTP_H
#include "io.h"
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 2048
#define RECV_SIZE 65534

const int IPV4 = 1;
const int IPV6 = 0;

char* send_request(std::string, std::string, int*, int);
char* send_request_tls(std::string, std::string, int *, int);

#endif //LABGET_HTTP_H
