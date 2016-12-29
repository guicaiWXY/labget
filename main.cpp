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
    int status;
    status = curl("http://www.sina.com.cn");
//    status = curl("http://www.sjtu.edu.cn");
//    status = curl("http://www.fudan.edu.cn/2016/index.html");
}



//void test() {
//    std::string s("this subject has a submarine as a subsequence");
//    std::smatch m;
//    std::regex e("\\b(sub)([^ ]*)");   // matches words beginning by "sub"
//
//    std::cout << "Target sequence: " << s << std::endl;
//    std::cout << "Regular expression: /\\b(sub)([^ ]*)/" << std::endl;
//    std::cout << "The following matches and submatches were found:" << std::endl;
//
//    while (std::regex_search(s, m, e)) {
//        for (auto x = m.begin(); x != m.end(); x++)
//            std::cout << x->str() << " ";
//        out(m.begin()->str());
//        std::cout << "--> ([^ ]*) match " << m.format("$2") << std::endl;
//        s = m.suffix().str();
//    }
//}

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
        out(string("HTTP used.\n"));
    } else if (correct == 2) {
        using_TLS = true;
        out(string("HTTPS used.\n"));
    }

    /*
     * Step 2: DNS transfer
     * */
    int *ip_addr ;
    if (using_ipv6) {
        ip_addr = dns_look_up_v6(segments[1], port);
        if (ip_addr == 0) {
            log("DNS transfer fails.");
            log();
        }
    } else {
        ip_addr = dns_look_up(segments[1], port);
        if (ip_addr == 0) {
            log("DNS transfer fails.");
            log();
            return -1;
        }
    }

    /*
     * Step 3: Send HTTP request
     * */
    int ip_port[2];
    ip_port[0] = *ip_addr;
    ip_port[1] = port;
    RESPONSE response;

    if (using_TLS) {

    } else {
        response = send_request(segments[1], segments[2], ip_port, IPV4);
    }
    if (response == NULL) {
        log("Get HTTP response error.\n");
        return -1;
    }

    /*
     * Step 4: parsing response
     * */
    std::vector<std::string> lines;
    int status = get_status_code(response, lines );
    if (status < 0) {
        log("Wrong status code: ");
        log(status);
        log();
        return -1;
    }
    switch (status) {
        // 2xx ok
        case 200:
            // OK
            break;
        // 3xx redirect
        case 301:
            // moved permanently
            break;
        case 302:
            // moved temporarily
            break;
        // 4xx client error
        case 400:
            // bad request
            break;
        case 404:
            //
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


    /*
     * clean up stage
     * */
//    free(ip_addr);
}

/*
 * Should always exit with status code 0
 * */
int main(int argc, char **args) {
    test_uri();
    exit(0);


//    if (argc != 1) {
//        std::cout << "Please include a desired url." << std::endl;
//    }
//    curl(args[0]);

    /*
     * Some test code
     * */
    string auth = "www.zhihu.com.cn";
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

//    char *rec_buffer = (char *)malloc(sizeof(char) * 10);
//    rec_buffer[4] = 0x01;
//    rec_buffer[5] = 0x02;
//
//#define GET(position) (((int)rec_buffer[position] << 8) + (int)(rec_buffer[position+1]))
//    // <<'s priority is lower than +  0.0
//    int tmp =((rec_buffer[4])) << 8 + (rec_buffer[4+1]);
//    int d0 = rec_buffer[4];
//    int debug = rec_buffer[5];
//    int tmp2 = GET(4);
//    log(tmp2);
////    GET_WORD(4);
//    log(tmp);

    using_TLS = false;
    using_ipv6 = false;
    unsigned int *ipv4_addr_ptr;
//    ipv4_addr_ptr = dns_look_up("zhihu.com", 80);
//    print_v4_addr(ipv4_addr_ptr);
    int ip_port[5];
//    ip_port[0] = *ipv4_addr_ptr;
    ip_port[1] = 80;
//    send_request("zhihu.com", "/index.html", ip_port, IPV4);

//    ipv4_addr_ptr = (unsigned int*)dns_look_up("www.sjtu.edu.cn", 80);
//    unsigned int addr = -285162887;
    unsigned int addr = 1996650698;

//    print_v4_addr(&addr);
//    print_v4_addr(ipv4_addr_ptr);
//    ip_port[0] = *ipv4_addr_ptr;
    ip_port[0] = addr;
//    send_request("www.sina.com.cn", "/", ip_port, IPV4);
    send_request("www.sjtu.edu.cn", "/", ip_port, IPV4);


//    if (curl("https://zhihu.com") == -1) {
//        exit(0);
//    }

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