//
// Created by wxy on 12/14/16.
//
#include <strings.h>
#include <arpa/inet.h>
#include <cstring>
#include "http.h"

void show_buffer(char *recv, unsigned int len) {
    log("receive ");
    log(len);
    if (len < RECV_SIZE) {
        recv[len+1] = 0;
        out(recv);
    }
}
void fill_buffer(char *buf_ptr, std::string str) {
    int len = str.length();
    if (len > BUFFER_SIZE) {
        log("HTTP request content too long.\n");
    }
    const char *cstr = str.c_str();
    memcpy(buf_ptr, cstr, len);
//    *buf_ptr += len;
}
//void buffer_end_line() {
//
//}
//
//#define END_LINE    \
//    *(s_b++) = 0x0d;    \
//    *(s_b++) = 0x0a;


/*
 * Return a string of content user required
 * */
char *send_request(std::string host, std::string request, int *ip_and_port, int version) {
    int sockfd;
    unsigned int portno, n;
    portno = version == IPV4 ? ip_and_port[1] : ip_and_port[4];
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    struct hostent *server;
    const char *hostname = host.c_str();
    char recv_buf[RECV_SIZE];
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
//        char *s_b = send_buf;
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
            log("Error when reading from socket.\n");
        }

        while ((n = read(sockfd, recv_buf, RECV_SIZE)) != 0) {
            show_buffer(recv_buf, n);
            out();
        }
        close(sockfd);
        log("\ncontent ends.\n");

    } else {


    }
}

char *send_request_tls(std::string host, std::string request, int *ip_and_port, int version) {

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