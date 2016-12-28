//
// Created by admin on 2016/12/11.
//

#ifndef LABGET_URI_H
#define LABGET_URI_H

#include "io.h"
#include <pcre.h>
#include <regex>

using std::string;
using std::regex;
using std::smatch;

int parse_uri(const char* uri, string[]);
int name_or_ipaddr(string, int* , string &);
#endif //LABGET_URI_H
