//
// Created by admin on 2016/12/11.
//

#ifndef LABGET_IO_H
#define LABGET_IO_H
#include "stdio.h"
#include <string>

void log(std::string);
void log(char * );
void log(int x);
void log(long x);
void log(unsigned long x);
void log(void);
void out(std::string);
void out(char *s);
void out(int);
void out(void);
#endif //LABGET_IO_H
