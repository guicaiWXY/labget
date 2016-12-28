//
// Created by admin on 2016/12/11.
//
#include "io.h"

void log(std::string s) {
//    std::cerr << s;
    fprintf(stderr, "%s", s.c_str());
}
void log(char *s) {
    fprintf(stderr, "%s", s);
}
void log(int x) {
    fprintf(stderr, "%d", x);
}
void log(void) {
    fprintf(stderr, "\n");
}
void out(std::string s) {
//    std::cout << s;
    fprintf(stdout, "%s", s.c_str());
}
void out(char *s) {
    fprintf(stdout, "%s", s);
}
void out(int x) {
    fprintf(stdout, "%d", x);
}
void out(void) {
    fprintf(stdout, "\n");
}

