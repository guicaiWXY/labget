//
// Created by wxy on 12/14/16.
//

#ifndef LABGET_HTTP_H
#define LABGET_HTTP_H
#include "io.h"
#include "gzip.h"
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#define BUFFER_SIZE 2048
#define RECV_SIZE 65534
#define RECV_BUFFER_SIZE (65536*64)

const int IPV4 = 1;
const int IPV6 = 0;

struct sized_buffer {
    char *buffer;
    unsigned long size;
};
struct response {
    struct sized_buffer header;
    struct sized_buffer content;
};

typedef struct sized_buffer SIZED_BUF;
typedef struct response * RESPONSE;

RESPONSE send_request(std::string, std::string, int*, int);
RESPONSE send_request_tls(std::string, std::string, int *, int);
int get_status_code(RESPONSE, std::vector<std::string> & lines);

#endif //LABGET_HTTP_H
