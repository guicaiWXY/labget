//
// Created by wxy on 12/14/16.
//
#include <strings.h>
#include <arpa/inet.h>
#include <cstring>
#include "http.h"

void show_content(char *recv, unsigned long len) {
    for (unsigned long i = 0; i < len; i++) {
        fprintf(stdout, "%c", recv[i]);
    }
}

void show_buffer(char *recv, unsigned long len) {
//    log();
//    log("receive ");
//    log(len);
    log();
    if (len < RECV_SIZE) {
        char x = recv[len + 1];
        recv[len] = 0;
        log(recv);
        recv[len] = x;
    }
}

void fill_buffer(char *buf_ptr, std::string str) {
    int len = str.length();
    if (len > BUFFER_SIZE) {
        log("HTTP request content too long.\n");
    }
    const char *cstr = str.c_str();
    memcpy(buf_ptr, cstr, len);
}

void split_string(const std::string &s, std::vector<std::string> &v, const std::string c) {
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    int c_size = c.size();
    while (std::string::npos != pos2) {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c_size;
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
}

// timeout is in ms
int set_socket_time_out(int sockfd, long timeout) {
    struct timeval timeo;
    socklen_t len = sizeof(timeo);
    timeo.tv_sec = timeout/1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1) {
        log("set socket connection time out error.\n");
        return -1;
    }
    return 0;
}
int set_socket_time_out1(int sockfd, long timeout) {
    struct timeval timeo;
    socklen_t len = sizeof(timeo);
    timeo.tv_sec = timeout/1000;
    timeo.tv_usec = timeout % 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) < 0) {
        log("set socket receive time out error.\n");
        return -1;
    }
    return 0;
}
/*
 * Return header and content content user required
 * */
RESPONSE send_request(std::string host, std::string request, int *ip_and_port, int version) {
    int sockfd;
    unsigned int portno, n;
    portno = version == IPV4 ? ip_and_port[1] : ip_and_port[4];
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    struct hostent *server;
    const char *hostname = host.c_str();
    char *recv_buf = (char *) malloc(sizeof(char) * RECV_BUFFER_SIZE);
    char send_buf[BUFFER_SIZE];

    // TCP
    if (version == IPV4) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            log("Error when establishing TCP connection.\n");
            return NULL;
        }
        // set connection timeout 2000ms
        if ( set_socket_time_out(sockfd, 2000) )
            return NULL;

        /* build the server's Internet address with given address */
        bzero(&server_addr, sizeof(sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = ip_and_port[0];       // already converted to net byte order (little endian)
        server_addr.sin_port = htons(portno);

        /* connect  */
        if (connect(sockfd, (sockaddr *) &server_addr, sizeof(struct sockaddr_in)) < 0) {
            if (errno == EINPROGRESS) {
                log("connection timeout.\n");
            }else {
                log("TCP connection failed.\n");
            }
            return NULL;
        }
        bzero(send_buf, BUFFER_SIZE);
        std::string head;
        if (request.find("/") == 0)
            head = "GET " + request + " HTTP/1.1\r\n";
        else
            head = "GET /" + request + " HTTP/1.1\r\n";
        // need to print header line
        log(head);

        head += "Host: " + host + "\r\n";
        head += "Connection: keep-alive\r\n";
        head += "Upgrade-Insecure-Requests: 1\r\n";
        head += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.71 Safari/537.36\r\n";
        head += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n";
        head += "Accept-Encoding: gzip\r\n"; //, deflate, sdch, br
        head += "\r\n";

        fill_buffer(send_buf, head);

        n = write(sockfd, send_buf, head.length());
        if (n < 0) {
            log("Error when sending http request.\n");
            return NULL;
        }

        /*
         * Read response
         * */
        if (set_socket_time_out1(sockfd, 1000))
            return NULL;

        unsigned long sum = 0;
        char head_buf[BUFFER_SIZE];
        n = read(sockfd, head_buf, BUFFER_SIZE);
        if (n == 0 - 1 | n == 0) {
            log("Empty Head.\n");
            return NULL;
        }
        char *x = "\r\n\r\n";
        head_buf[n] = 0;
        std::string headers = std::string(head_buf);

        unsigned long tmp;
        sum = 0;
        bzero(recv_buf, RECV_BUFFER_SIZE);

//        char * _x = std::find(head_buf, x, 4);
        int index = headers.find(x);
        if ( n == index + 4) {
            // ok
        } else {
            // move abundant bytes
            char * _x = head_buf + index;
            memcpy(recv_buf, _x + 4, head_buf + n - _x - 4);
            sum = head_buf + n - _x - 4;
            n = _x + 4 - head_buf;
        }

//        show_buffer(head_buf, n);

        while (true) {
            tmp = read(sockfd, recv_buf + sum, RECV_BUFFER_SIZE);
            if (tmp == 0 | tmp == 0 - 1)
                break;
            log(tmp);
            log();
//            show_buffer(recv_buf + sum, tmp);

            sum += tmp;
            if (sum > RECV_BUFFER_SIZE) {
                log("Buffer Overflow.");
                break;
            }
            std::string segment = std::string(recv_buf + sum - tmp);
            int in = segment.find("</html>");
            if (segment.size() > 15 & in > (int) (segment.size() - 15)) {
                break;
            }
        }
        if (sum == 0 | sum == 0 - 1) {
            log("Empty Content.\n");
            return NULL;
        }
        log(sum);
        log();

//        show_content(recv_buf, sum);

        // fill return data
        SIZED_BUF response_header = {
                head_buf, n
        };
        SIZED_BUF response_content = {
                recv_buf, sum
        };

        RESPONSE response = (RESPONSE) malloc(sizeof(struct response));
        response->header = response_header;
        response->content = response_content;

        close(sockfd);
        return response;
    } else {
        // IPV6

    }
}

RESPONSE send_request_tls(std::string host, std::string request, int *ip_and_port, int version) {

}

//void parse_header(RESPONSE response) {
//
//}

void show_header(std::vector<std::string> lines) {
//    log("Http Response:\n");
    log(lines[0] + "\n");
//    log();
//    log("Headers are:\n");

//    int end = lines.size()-1;
//    for (int i=1; i < end; i++) {
//        log(lines[i] + "\n");
//    }
}

int get_port(std::string);  // in uri.cpp

int get_hex(char *start, char *ret) {
    int sum = 0;
    int len = ret - start;
    int tmp;
    for (int i = 0; i < len; i++) {
        char x = start[i];
        if (x <= 0x3a)
            tmp = x-'0';
        else
            tmp = (x - 'a') + 10;
        sum = sum * 16 + tmp;
    }
    return sum;
}

int get_status_code(RESPONSE response, std::vector<std::string> &lines) {
    char *header = response->header.buffer;
    header[response->header.size] = 0;
    std::string head = std::string(header);
    split_string(head, lines, "\r\n");
//    log(lines.at(lines.size()-1));

    show_header(lines);

    std::vector<std::string> codes;
    split_string(lines[0], codes, " ");
    if (codes[0].compare("HTTP/1.1")) {
        log("Wrong HTTP response version:" + codes[0]);
        return -1;
    }
    int status = get_port(codes[1]);
    return status;
}

void decompress(SIZED_BUF &entity, std::vector<std::string> lines) {
    bool truncated = false;
    bool gziped = false;
    int len = lines.size();
    std::string enc = "Content-Encoding: ";
    std::string tran = "Transfer-Encoding: ";
    int index = -1;
    int index1 = -1;
    // get response encoding
    for (int i = 1; i < len - 1; i++) {
        if (lines[i].find(enc, 0) == 0) {
            index = i;
            if (!lines[i].substr(enc.length()).compare("gzip")) {
                gziped = true;
            }
            if (index1 >= 0)
                break;
        } else if (lines[i].find(tran) == 0) {
            index1 = i;
            if (!lines[i].substr(tran.length()).compare("chunked")) {
                truncated = true;
            }
            if (index >= 0)
                break;
        }
    }

    if (gziped) {
        // encoding with gzip
        Byte *buf = (Byte *) malloc(sizeof(char) * RECV_SIZE * 16);
        unsigned long size;
        if (!truncated) {
            if (gzdecompress((Byte *) entity.buffer, entity.size, buf, &size) == 0) {
//                log("size = ");
//                log(size);
//                log();
                entity.buffer = (char *) buf;
                entity.size = (long) size;
            } else {
                log("GZ decompress failed.\n");
            }
        } else {
            unsigned long sum = 0;
            char * ret = "\r\n";
            char * tmp_index;
            char * start_index = entity.buffer;
            while (true) {
                std::string origin = std::string(start_index);
                int index = origin.find(ret);
                tmp_index = start_index + index;
                int s = get_hex(start_index, tmp_index);

                log("chunked size = ");
                log(s);
                log("\n");

                start_index = tmp_index + 2;
                if (s == 0)
                    break;
                else {
                    if (gzdecompress((Byte*)start_index, s, buf + sum, &size) == 0) {
                        start_index += s + 2;   // \r\n for chunk end
                        sum += size;
                    } else {
                        log("GZ decompress failed.\n");
                        return;
                    }
                }
            }
            log(sum);
            entity.buffer = (char *) buf;
            entity.size = sum;
        }
    } else {
        // plain text
        return;
    }
}


//#define BUFSIZE 1024
//
///*
// * error - wrapper for perror
// */
//void error(char *msg) {
//    perror(msg);
//    exit(0);
//}
//
//int main(int argc, char **argv) {
//    int sockfd, portno, n;
//    struct sockaddr_in serveraddr;
//    struct hostent *server;
//    char *hostname;
//    char buf[BUFSIZE];
//
//    /* check command line arguments */
//    if (argc != 3) {
//        fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
//        exit(0);
//    }
//    hostname = argv[1];
//    portno = atoi(argv[2]);
//
//    /* socket: create the socket */
//    sockfd = socket(AF_INET, SOCK_STREAM, 0);
//    if (sockfd < 0)
//        error("ERROR opening socket");
//
//    /* gethostbyname: get the server's DNS entry */
//    server = gethostbyname(hostname);
//    if (server == NULL) {
//        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
//        exit(0);
//    }
//
//    /* build the server's Internet address */
//    bzero((char *) &serveraddr, sizeof(serveraddr));
//    serveraddr.sin_family = AF_INET;
//    bcopy((char *)server->h_addr,
//          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
//    serveraddr.sin_port = htons(portno);
//
//    /* connect: create a connection with the server */
//    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0)
//        error("ERROR connecting");
//
//    /* get message line from the user */
//    printf("Please enter msg: ");
//    bzero(buf, BUFSIZE);
//    fgets(buf, BUFSIZE, stdin);
//
//    /* send the message line to the server */
//    n = write(sockfd, buf, strlen(buf));
//    if (n < 0)
//        error("ERROR writing to socket");
//
//    /* print the server's reply */
//    bzero(buf, BUFSIZE);
//    n = read(sockfd, buf, BUFSIZE);
//    if (n < 0)
//        error("ERROR reading from socket");
//    printf("Echo from server: %s", buf);
//    close(sockfd);
//    return 0;
//}

