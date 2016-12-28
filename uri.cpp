//
// Created by admin on 2016/12/11.
//
#include "uri.h"
//#include "pcre.h"

using namespace std;
int get_port(string port) {
    int i = 0;
    int len = port.length();
    int sum = 0;
    for (; i < len; i++) {
        sum = sum * 10 + (int) (port.at(i) - '0');
    }
    return sum;
}

int in_0_to_255(string segment) {
//    int x = segment.c_str()
    int sum = get_port(segment);
    if (sum < 0 || sum > 255)
        return -1;
    return sum;
}

void alert_port(void) {
    out(string("The port number is out of range."));
}

void alert_ipv4_addr(void) {
    out(string("Wrong ipv4 format."));
}

int judge_port_range(int *port, string port1) {
    if (!port1.empty()) {
        if (port1.length() > 5) {
            // must be out of range
            alert_port();
            return -1;
        }
        *port = get_port(port1);
        if (port1.length() == 5 && *port >= 0) {
            // also out of range
            alert_port();
            return -1;
        }
    } else {
        // default port number for HTTP protocol
        *port = 80;
    }
    return 0;
}

int parse1(string authority, string &auth, int *port, string port1) {
    int index = authority.find(':');
    if (index == -1) {
        auth = authority;
        *port = 80;
    } else {
        auth = authority.substr(0, index);
//        log("parsed auth = " + auth);
        if (judge_port_range(port, port1) == -1)
            return -1;
    }
    return 0;
}

int parse_uri(const char *uri, string segments[]) {
//    pcre *re;
//    const char *error;
//    int erroffset;
//    int ovector[30];
//    int options = 0;

    std::string uri_str(uri);
    log("URI to parse is : " + uri_str + "\n");

    char *uri_regular_expression = "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)([?]([^#]*))?(#(.*))?";
//    if ((pcre_compile(uri_regular_expression, options, &error, &erroffset, NULL)) == NULL) {
//        log("Regex compilation failed");
//        log();
//    }
//
//    if(pcre_exec(re, NULL, uri, uri_str.length(), 0, 0, ovector, 30) >= 0)
//        log("Match.");
//    else
//        log("Not Found.");
//    log();


    std::regex uri_format_windows("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)([?]([^#]*))?(#(.*))?", std::regex::extended);
//    std::regex uri_format_linux("http[s]?://*", regex::basic);
    //      (//([^/#]*))  ([^#]*)([]([^#]*))(#(.*))
//    regex uri_format = uri_format_linux;
    regex uri_format = uri_format_windows;
    std::smatch m;

    // match regex expression
    int index = 0;
    std::regex_search(uri_str, m, uri_format);

    // log out parsing result
    for (auto x: m) {
        segments[index++] = x.str();

//        log(index++);
//        log(x.str() + "\n");
    }

//    out(string("\n"));
//    std::regex r("^http([s]?)://([^/?#]*)?([^?#]*)?([?]([^#]*))?(#(.*))?", std::regex::extended);
//    std::regex_search(uri_str, m, r);
//    int index = 0;
//    for (auto x: m) {
//        out("$");
//        out(index++);
//        out(" :" + x.str() + "\n");
//    }
//    out("$1" + m.format("$1") + "\n");

    // judge by url scheme
    if (segments[2].length() == 0 || segments[4].length() == 0)
        return 0;
    if (segments[2].compare("http")) {
//         http support
        return 1;
    } else if (segments[2].compare("https")) {
        // https support
        return 2;
    }
    return 0;
}

/*
 * return 0 for name
 * return 1 for address
 * return -1 for error
 * */
int name_or_ipaddr(string authority, int *ipv4_addr_port, string &auth) {
    log("authority is : " + authority);
    log();
    regex r("(([0-9]+).)?(([0-9]+).)?(([0-9]+).)?([0-9]+)?([^:]*)?(:([0-9]+))?(.*)?");
    smatch m;
    regex_search(authority, m, r);
    int index = 0;
    string tmp[12];
    if (!m.empty()) {
        for (auto x: m) {
            tmp[index++] = x.str();
//            log(string("$"));
//            log(index);
//            log(": " + x.str() + "\n");
        }
    }
    int ret = 0;

    /*
     * $8 : redundant content after an ipv4 addr or domain name
     * $10: port num
     * $11: should be empty
     * */
    // ipv6 example: CDCD:910A:2222:5498:8475:1111:3900:2020
    if (!tmp[11].empty()) {
        // there is something redundant after port number
        // 1.2.3.4:5abd or fdu.com:80lalala
        return -1;
    }

    if (!(tmp[2].empty() && tmp[4].empty() && tmp[6].empty() && tmp[7].empty())) {
        if (tmp[8].empty()) {
            // 4 seg in 0-255
            if ((ipv4_addr_port[0] = in_0_to_255(tmp[2])) != -1 && (ipv4_addr_port[1] = in_0_to_255(tmp[4])) != -1 &&
                (ipv4_addr_port[2] = in_0_to_255(tmp[6])) != -1 && (ipv4_addr_port[3] = in_0_to_255(tmp[7])) != -1) {
                ret = 1;
                // port number 0 - 65535
                if (judge_port_range(ipv4_addr_port + 4, tmp[10]) == -1)
                    return -1;
            } else {
                alert_ipv4_addr();
                return -1;
            }

        } else {
            // not ipv4
            if (parse1(authority, auth, ipv4_addr_port + 4, tmp[10]) == -1)
                return -1;
        }
    } else {
        if (parse1(authority, auth, ipv4_addr_port + 4, tmp[10]) == -1)
            return -1;
    }
    return ret;
}