//
// Created by wxy on 12/14/16.
//
#include <strings.h>
#include <arpa/inet.h>
#include <cstring>
#include "http.h"

void show_content(char *recv, unsigned long len) {
    for (unsigned long i = 0; i<len; i++) {
        fprintf(stderr, "%c", recv[i]);
    }
    log();
}
void show_buffer(char *recv, unsigned long len) {
    log();
    log("receive ");
    log(len);
    log();
    if (len < RECV_SIZE) {
        char x = recv[len+1];
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
void split_string(const std::string & s, std::vector<std::string> & v, const std::string c) {
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    int c_size = c.size();
    while (std::string::npos != pos2) {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c_size;
        pos2 = s.find(c,pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
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
    char* recv_buf = (char *)malloc(sizeof(char)*RECV_BUFFER_SIZE);
    char send_buf[BUFFER_SIZE];

    // TCP
    if (version == IPV4) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            log("Error when establishing TCP connection.\n");
            return NULL;
        }

        /* build the server's Internet address with given address */
        bzero(&server_addr, sizeof(sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = ip_and_port[0];       // already converted to net byte order (little endian)
        server_addr.sin_port = htons(portno);

        /* connect  */
        if (connect(sockfd, (sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0) {
            log("TCP connection failed.\n");
            return NULL;
        }
        bzero(send_buf, BUFFER_SIZE);
        std::string head;
        if (request.find("/") == 0)
            head = "GET " + request + " HTTP/1.1\r\n";
        else
            head = "GET /" + request + " HTTP/1.1\r\n";
        head += "Host: " + host + "\r\n";
        head += "Connection: keep-alive\r\n";
        head += "Upgrade-Insecure-Requests: 1\r\n";
        head += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.71 Safari/537.36\r\n";
        head += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n";
        head += "Accept-Encoding: gzip\r\n"; //, deflate, sdch, br
        head += "\r\n";

        fill_buffer(send_buf, head);
        out(head);

        n = write(sockfd, send_buf, head.length());
        if (n < 0) {
            log("Error when sending http request.\n");
            return NULL;
        }

        unsigned long sum = 0;
        char head_buf[BUFFER_SIZE];
        n = read(sockfd, head_buf, BUFFER_SIZE);
        if (n == 0-1 | n == 0)
        {
            log("Empty Head.\n");
            return NULL;
        }
//        show_buffer(head_buf, n);

        unsigned long tmp;
        sum = 0;
        while (true) {
//            n = read(sockfd, recv_buf+sum, RECV_SIZE);
            tmp = read(sockfd, recv_buf+sum, RECV_BUFFER_SIZE);
            if (tmp == 0 | tmp == 0-1)
                break;
            log(tmp);
            log(" ");
//            show_buffer(recv_buf + sum, n);
            sum += tmp;
            if (sum > RECV_BUFFER_SIZE) {
                log("Buffer Overflow.");
                break;
            }
        }
        if (sum == 0 | sum == 0-1) {
            log("Empty Content.\n");
            return NULL;
        }

//        show_content(recv_buf, sum);
        SIZED_BUF response_header = {
                head_buf, n
        };
        SIZED_BUF response_content = {
                recv_buf, sum
        };

        RESPONSE response = (RESPONSE)malloc(sizeof(struct response));
        response->header = response_header;
        response->content = response_content;

        close(sockfd);
        log("\ncontent ends.\n");
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
    out("Http Response:\n");
    out("First line is: " + lines[0] + "\n");
    out("Headers are:\n");
    int end = lines.size()-1;
    for (int i=1; i < end; i++) {
        out(lines[i]);
        out();
    }
}

int get_port(std::string);  // in uri.cpp
int get_status_code(RESPONSE response, std::vector<std::string> &lines) {
    char * header = response->header.buffer;
    header[response->header.size] = 0;
    std::string head = std::string(header);
//    std::vector<std::string> lines;
    split_string(head, lines, "\r\n");
    log(lines.at(lines.size()-1));
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

void decompress(SIZED_BUF & entity, std::vector<std::string> lines) {
    int len = lines.size();
    std::string enc = "Content-Encoding: ";
    int index = 0;

    for (int i = 1; i < len-1; i++) {
        if (lines[i].find_first_of(enc, 0) == 0) {
            index = i;
            break;
        }
    }
    if (index > 0) {
        std::string encode = lines[index].substr(enc.length());
        if (!encode.compare("gzip")) {
            // encoding with gzip
            Byte *buf = (Byte*)malloc(sizeof(char)*RECV_SIZE * 16);
            unsigned long size;
            if (gzdecompress((Byte *)entity.buffer, entity.size, buf, &size) == 0) {
                log(size);
            }
            else {
                log("GZ decompress failed.\n");
            };
        } else {
            // plain text
        }
    } else {
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

