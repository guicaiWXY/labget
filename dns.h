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

//#pragma comment(lib, "ws2_32.lib")

extern const unsigned int UDP_PORT;
extern const char* DNS_SERVER_ADDRESS;
extern const unsigned short DNS_PORT;

enum TYPE {
    A = 0x01,       // ipv4 host addr
    NS = 0x02,      // authoritive name server
    MD = 0x03,      // mail destination
    MF = 0x04,      // mail forwarder
    CNAME = 0x05,   // canonical name
    SOA = 0x06,     // start of a zone of authority
    MB = 0x07,      // mailbox domain name
    MG,             // mail group member
    MR,             // mail rename domain name
    _NULL,
    WKS,            // well known service description
    PTR,
    HINFO,
    MINFO,
    MX,
    TXT,
    AAAA = 0x1c,    // ipv6 host addr
};
enum CLASS {
    IN = 0x01,   // The Internet
    CS,          // CSNET class
    CH,          // CHAOS class
    HS           // Hesiod
};

struct resource_record {
    std::string* name;
    TYPE type;
    CLASS klass;
    unsigned int ttl;
    unsigned short rdlength;
    char * data;
    std::string* cname;
};

typedef struct resource_record RR;

int* dns_look_up(std::string);
int* dns_look_up_v6(std::string);

int* dns_look_up(std::string, bool);
void log_ip2(unsigned char*addr_ptr);
void log_ip1(int *addr_ptr);
#endif //LABGET_DNS_H
