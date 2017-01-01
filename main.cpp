#include "labget.h"

bool using_ipv6 = false;
bool using_TLS = false;

extern const int IPV4;
extern const int IPV6;

void print_v4_addr(unsigned int *addr_ptr) {
    unsigned int addr = *addr_ptr;
    fprintf(stderr, "Address is : %d.%d.%d.%d\n", (addr >> 0) & 0xff,
            (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff);
}

void test_uri() {
//    using_ipv6 = true;
    int status;
//    status = curl("http://www.sina.com.cn");
//    status = curl("http://www.sjtu.edu.cn");
//    status = curl("http://www.fudan.edu.cn/2016/index.html");
    status = curl("http://xiami.com");
}

std::string new_uri(std::vector<std::string> lines) {
    int i = 0;
    int len = lines.size();
    std::string location = "Location: ";
    for (; i < len - 1; i++) {
        if (lines[i].find(location) == 0) {
            return lines[i].substr(location.length());
        }
    }
    return "";
}

void log_ip(int *_addr) {
    int addr = _addr[0];
    fprintf(stderr, "%d.", (unsigned char)addr & 0xff);
    fprintf(stderr, "%d.", (unsigned char)(addr >> 8)&0xff);
    fprintf(stderr, "%d.", (unsigned char)(addr >> 16)&0xff);
    fprintf(stderr, "%d", (unsigned char)(addr >> 24)&0xff);
    log();
}
void log_ip2(unsigned char*addr_ptr) {
    unsigned short x = addr_ptr[0] + (addr_ptr[1] << 8);
    if (x != 0)
        fprintf(stderr, "%04x", x);
}
void log_ip1(int *addr_ptr) {
    if (addr_ptr[0] == 4) {
        log_ip(addr_ptr + 1);
    } else {
//        fprintf(stderr, "%02x", ((unsigned char *)addr_ptr)[15]);
//        fprintf(stderr, "%02x:", ((unsigned char *)addr_ptr)[14]);
//        fprintf(stderr, "%02x", ((unsigned char *)addr_ptr)[13]);
//        fprintf(stderr, "%02x:", ((unsigned char *)addr_ptr)[12]);
//        fprintf(stderr, "%02x", ((unsigned char *)addr_ptr)[11]);
//        fprintf(stderr, "%02x:", ((unsigned char *)addr_ptr)[10]);
//        fprintf(stderr, "%02x", ((unsigned char *)addr_ptr)[9]);
//        fprintf(stderr, "%02x:", ((unsigned char *)addr_ptr)[8]);
//        fprintf(stderr, "%02x", ((unsigned char *)addr_ptr)[7]);
//        fprintf(stderr, "%02x:", ((unsigned char *)addr_ptr)[6]);
//        fprintf(stderr, "%02x", ((unsigned char *)addr_ptr)[5]);
//        fprintf(stderr, "%02x:", ((unsigned char *)addr_ptr)[4]);
//        fprintf(stderr, "%02x", ((unsigned char *)addr_ptr)[3]);
//        fprintf(stderr, "%02x:", ((unsigned char *)addr_ptr)[2]);
//        fprintf(stderr, "%02x", ((unsigned char *)addr_ptr)[1]);
//        fprintf(stderr, "%02x", ((unsigned char *)addr_ptr)[0]);
        for (int i = 0; i < 7; i++) {
            log_ip2((unsigned char*)(addr_ptr + 2*i));
            log(":");
        }
        log_ip2((unsigned char*)(addr_ptr + 2*7));
        log();
    }
}

int curl(const char *uri) {
    /*
    * Step 1: parsing URI
    * */
    string segments[3];
    int port;
//    int correct = parse_uri(uri, segments);
    int correct = parse_new(uri, segments, &port);
    if (!correct) {
        // throw an exception
        log(string("Wrong uri format, system exits.\n"));
        return -1;
    }
    if (correct == 1) {
//        out(string("HTTP used.\n"));
    } else if (correct == 2) {
        using_TLS = true;
//        out(string("HTTPS used.\n"));
    }

    /*
     * Step 2: DNS transfer
     * */
    int *ip_addr;
    if (using_ipv6) {
        ip_addr = dns_look_up_v6(segments[1]);
        if (ip_addr == 0) {
            log("DNS transfer fails.");
            log();
            return -1;
        } else {
            log_ip1(ip_addr);
        }
    } else {
        ip_addr = dns_look_up(segments[1]);
        if (ip_addr == 0) {
            log("DNS transfer fails.");
            log();
            return -1;
        } else {
            log_ip(ip_addr);
        }
    }

    /*
     * Step 3: Send HTTP request
     * */
    RESPONSE response;
    if (!using_ipv6) {
        int ip_port[2];
        ip_port[0] = ip_addr[0];
        ip_port[1] = port;
        response = send_request(segments[1], segments[2], ip_port, IPV4);
    } else if (ip_addr[0] == 4) {

        int ip_port[2];
        ip_port[0] = ip_addr[1];
        ip_port[1] = port;
        if (using_TLS) {

        } else {
            response = send_request(segments[1], segments[2], ip_port, IPV4);
        }
    } else if (ip_addr[0] == 6) {
        int ip_port[5];
        memcpy(ip_port, ip_addr, 16);
        ip_port[4] = port;
        if (using_TLS) {

        } else {
            // TODO
            response = send_request(segments[1], segments[2], ip_port, IPV6);
        }
    }

    if (response == NULL) {
        log("Get HTTP response error.\n");
        return -1;
    }

    /*
     * Step 4: parsing response
     * */
    std::vector<std::string> lines;
    int status = get_status_code(response, lines);
    if (status < 0) {
        log("Wrong status code: ");
        log(status);
        log();
        return -1;
    }
    std::string uri_new;
    bool ok = false;
    switch (status) {
        // 2xx ok
        case 200:
            // OK
            decompress(response->content, lines);
            if (response->content.size > 0)
                ok = true;
            break;
            // 3xx redirect
        case 301:
            // moved permanently
            uri_new = new_uri(lines);
            if (uri_new.compare(""))
                curl(uri_new.c_str());
            exit(0);
//            break;
        case 302:
            // moved temporarily
            uri_new = new_uri(lines);
            uri_new = segments[0] + "://" + segments[1] + "/" + uri_new;
            curl(uri_new.c_str());
            exit(0);
//            break;
            // 4xx client error
        case 400:
            // bad request
            break;
        case 404:
            //
            exit(0);
            break;
            // 5xx server error
        case 500:
            //
            break;


        default:
            log("Unsupported Status code.\n");
            break;
    }

    /*
     * Step 5: Show response
     * */
    if (ok)
        show_content(response->content.buffer, response->content.size);

    /*
     * clean up stage
     * */
//    free(ip_addr);
}

/*
 * Should always exit with status code 0
 * */
int main(int argc, char **args) {
//    test_uri();
//    exit(0);

    if (argc != 2) {
        std::cout << "Please include a desired url." << std::endl;
    }

    using_TLS = false;
    using_ipv6 = false;
    using_ipv6 = true;

    if (curl(args[1]) == -1) {
        exit(0);
    }

    exit(0);
}
//    print_parsed_info(addr, auth);
//    log(name_or_ipaddr("10.11.11.11:1234", addr, auth));
//    log();
//    print_parsed_info(addr, auth);
//    log(name_or_ipaddr("10.11.112.212", addr, auth));
//    log();
//    print_parsed_info(addr, auth);
//    log(name_or_ipaddr("1.2.3.4.5:212", addr, auth));
//    log();
//    print_parsed_info(addr, auth);
//    log(name_or_ipaddr("mail.fudan.edu.cn", addr, auth));
//    log();
//    print_parsed_info(addr, auth);
//    log(name_or_ipaddr("mail.fudan.edu.cn:8080", addr, auth));
//    log();
//    print_parsed_info(addr, auth);
//    log(name_or_ipaddr("mail.fudan.edu.cn:8080abcd", addr, auth));
//    log();
//void print_parsed_info(int *addr, string auth) {
//    int i = 0;
//    log("ip addr is: ");
//    for (; i < 4; i++) {
//        log(addr[i]);
//        log(" ");
//    }
//    log();
//    log("Port num is: ");
//    log(addr[4]);
//    log();
//    log("Auth is: " + auth + "\n");
//}
//    int tmp_find = -1;
//    for (int i=0; i<5; i++) {
//        log("before find x = ");
//        log( tmp_find );
//        log();
//        tmp_find = auth.find('.', tmp_find+1);
//        log("afer find x = ");
//        log(auth.substr(0,tmp_find));
//        log(tmp_find);
//        log();
//        log();
//    }
//    log((int)sizeof(short));
