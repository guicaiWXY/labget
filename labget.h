//
// Created by wxy on 2016/12/10.
//

#ifndef LABGET_LABGET_H
#define LABGET_LABGET_H
#include <iostream>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

// tool files
#include "dns.h"
#include "uri.h"
#include "io.h"
#include "http.h"
//#include "gzip.h"
//#include ""

// MACROs

//using namespace std;



// global variables
const char* DNS_SERVER_ADDRESS = "202.120.224.26";
//const unsigned int UDP_PORT = 8888;
const unsigned short DNS_PORT = 53;

int curl(const char*uri);

#endif //LABGET_LABGET_H
