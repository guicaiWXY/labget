//
// Created by admin on 2016/12/11.
//

#include "dns.h"

#define BUFFER_SIZE 512

#define PUT_2 put_word_to_buffer(&s_b, &send_size, _word)
#define PUT_1 put_byte_to_buffer(&s_b, &send_size, _byte)
#define PUT_S put_string_to_buffer(&s_b, &send_size, section)

#define put_byte_to_buffer(buffer, len, _byte)      \
    **(buffer) = _byte;                             \
    *len = *len+1;                                  \
    *(buffer) += 1;

#define put_word_to_buffer(buffer, len, _word)      \
    **(buffer) = (char)(((_word) >> 8) & 0xff);     \
    *(*(buffer)+1) = (char)((_word) & 0xff);        \
    *len = *len + 2;                                \
    *(buffer) += 2;

#define put_string_to_buffer(buffer, len, string)   \
    sprintf(*(buffer), "%s", string.c_str());       \
    *(buffer) += string.length();                   \
    *len = *len + string.length();

#define GET_TYPE get_word_from_buffer(&r_b, &type)
#define GET_KLASS get_word_from_buffer(&r_b, &klass)
#define GET_TTL get_int_from_buffer(&r_b, &ttl)
#define GET_DATA_LENGTH get_word_from_buffer(&r_b, &data_length)

#define get_word_from_buffer(buffer_ptr, int_ptr)   \
    *(int_ptr) = GET_WORD1((*(buffer_ptr)));        \
    *(buffer_ptr) += 2;

#define get_int_from_buffer(buffer_ptr, int_ptr)    \
    *(int_ptr) = GET_WORD2((*(buffer_ptr)));        \
    *(buffer_ptr) += 4;

#define GET_WORD1(r_b) (((int)(r_b)[0] << 8) + (int)(r_b)[1])
#define GET_WORD2(r_b)  (((int)(r_b)[0]<<24) + ((int)(r_b)[1]<<16) + ((int)(r_b)[2]<<8) + (int)(r_b)[3])

#define DNS_ERROR (!!(rec_buffer[3] & 0x01))
#define DNS_RECUR_SUP (!(rec_buffer[3] & 0x80))
#define GET_QUESTIONS GET_WORD(4)
#define GET_ANSWERS GET_WORD(6)
#define GET_AUTHs GET_WORD(8)
#define GET_ADDS GET_WORD(10)

#define GET_WORD(position) (((int)rec_buffer[position] << 8) + (int)(rec_buffer[position+1]))

static size_t send_size;
static int random_transaction_id = rand();
static int len;

void log_hex(unsigned int hex) {
    fprintf(stderr, "%x", hex);
}

void show_buffer(char *buffer, int send_size) {
    log("Send Buffer is :\n");
    for (int i = 0; i < send_size; i++) {
        fprintf(stderr, "%02x ", (unsigned char) (*(buffer + i)));
    }
    log();
}

void show_buffer1(char *recv, int len) {
    log("Receive buffer is: \n");
    for (int i = 0; i < len; i++) {
        if (i > 0 & i % 16 == 0)
            log();
        fprintf(stderr, "%02x ", (unsigned char) *(recv + i));
    }
    log();
}

int get_a_name_length(char **name_start_ptr) {
    int len = 0;
    while (true) {
        unsigned int tmp = (unsigned char) *((*name_start_ptr) + len);
        if (tmp == 0) {
            *name_start_ptr += len + 1;
            return len + 1;
        } else if (tmp >= 0xc0) {
            *name_start_ptr += len + 2;
            return len + 2;;
        } else {
            len++;
            if (len > 1024) {
                log("Too long name, maybe some error occurs\n.");
            }
        }
    }
}


int *dns_look_up(std::string auth) {
    int *result_recursive = dns_look_up(auth, true);
    if (result_recursive == 0) {
        result_recursive = dns_look_up(auth, false);
    }
    return result_recursive;
}

int *dns_look_up(std::string auth, bool recursive) {
    using namespace std;
    int s;
    struct sockaddr_in addr;
    int addr_len = sizeof(struct sockaddr_in);

    char *send_buffer = new char[BUFFER_SIZE];
    char *rec_buffer = new char[BUFFER_SIZE];

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        log("udp socket error");
        return 0;
    }
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DNS_PORT);
    addr.sin_addr.s_addr = inet_addr(DNS_SERVER_ADDRESS);

    bzero(rec_buffer, sizeof(rec_buffer));

    send_size = 0;

    char _byte;
    short _word;
    char *s_b = send_buffer;

//                     Message Format
//                                    1  1  1  1  1  1
//      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                      ID                       |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                    QDCOUNT                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                    ANCOUNT                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                    NSCOUNT                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                    ARCOUNT                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

    /*
     * Part 1
     * Head Sector
     * */

    // BYTE 1 2
    // transaction id
    random_transaction_id++;    // global
    int tmp_id = random_transaction_id;
    _word = (short) random_transaction_id;
    PUT_2

    // BYTE 3
    // Flag 1st half
    // 0 0000 0 0 1
    _byte = recursive ? (char) 0x01 : (char) 0x00;
    PUT_1

    // BYTE 4
    // Flag 2nd half
    _byte = (char) 0x00;
    PUT_1

    // BYTE 5 6
    // Question Numbers
    _word = (short) 0x01;
    PUT_2

    // BYTE 7 8
    // Answer Numbers
    _word = (short) 0000;
    PUT_2

    // BYTE 9 10
    // Authority Answer Numbers
    _word = (short) 0x00;
    PUT_2

    // BYTE 11 12
    // Additional Answer Numbers
    _word = (short) 0x00;
    PUT_2

//                Question Section Format
//                                    1  1  1  1  1  1
//      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                                               |
//    /                     QNAME                     /
//    /                                               /
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                     QTYPE                     |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                     QCLASS                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

    /*
     * Part 2
     * Questions
     * */
    int start_find = -1;
    int tmp_find;
    string section = auth;
    // QNAME
    // split with "."
    while ((tmp_find = auth.find('.', start_find + 1)) != -1) {
        section = auth.substr(start_find + 1, tmp_find - start_find - 1);
        int tmp_len = section.length();
        if (tmp_len > 0x100) {
            return 0;
        }
        _byte = (char) tmp_len;
        PUT_1
        PUT_S
        start_find = tmp_find;
    }

    section = auth.substr(start_find + 1, auth.length() - start_find - 1);
    _byte = section.length();
    PUT_1
    PUT_S

    _byte = (char) 0x00;
    PUT_1

    // QTYPE
    // A type 0x0001
    _word = (short) 0x0001;
    PUT_2

    // QCLASS
    // InterNet(IN) 0x0001
    _word = (short) 0x0001;
    PUT_2

//    show_buffer(send_buffer, send_size);

    /*
     * No other part for DNS queries
     * */

    /*
     * Send Query Message to DNS Server
     * */
    sendto(s, send_buffer, send_size, 0, (struct sockaddr *) &addr, addr_len);

    /*
     * Receive Response Message from DNS Server
     * */
    len = recvfrom(s, rec_buffer, BUFFER_SIZE, 0, (struct sockaddr *) &addr, (socklen_t *) &addr_len);
    close(s);

    if (recursive && DNS_RECUR_SUP) {
        log("Recursive query not supported.\n");
        return 0;
    }
    if (DNS_ERROR) {
        log("DNS Response Code Error.\n");
    }

    // Check Transaction ID & Response Code
    if (GET_WORD(0) == (short) tmp_id) {
//        log("Transaction id ");
//        log_hex(tmp_id);
//        log(" Matched\n");
    } else {
        log("Query id is ");
        log_hex((short) tmp_id);
        log(" , While Response id is ");
        log_hex(*(short *) rec_buffer);
        log();
        return 0;
    }
//    log();
    int questions;
    int answers;
    int authorities;
    int additional;
    questions = GET_QUESTIONS;
    answers = GET_ANSWERS;
    authorities = GET_AUTHs;
    additional = GET_ADDS;
    log("Question : ");
    log(questions);
    log(" Answers : ");
    log(answers);
    log(" Authority RRs : ");
    log(authorities);
    log(" Additional RRs : ");
    log(additional);
    log();

    // for example
    // www.zhihu.com 's length is 3+1+5+1+3   = 13B
    // name: 3www5zhihu3com0    1+3+1+5+1+3+1 = 13B+2B
    int name_len = auth.length() + 2;

    char *r_b = rec_buffer + name_len + 12 + 4;

    int ipv4_addr_ptr;

    if (recursive) {
        /*
         * in recursive mode, I just parse each answer returned
         * and check if it is type A (which contains ip address)
         *
         * */
        int type;           // 2 B
        int klass;          // 2 B
        int ttl;            // 4 B
        int data_length;    // 2 B
        char *data = new char[BUFFER_SIZE];

        for (int j = 0; j < answers; j++) {
            int str_len = get_a_name_length(&r_b);
            GET_TYPE
            GET_KLASS
            GET_TTL
            GET_DATA_LENGTH
            memcpy(data, r_b, data_length);
            r_b += data_length;

            ipv4_addr_ptr = *(unsigned int *) data;
//            data[data_length] = 0;

            // According to types of the answer
            // Get Response Data
            switch (type) {
                case 0x01:
                    // A
                    // 4B in data is ipv4 addr
//                get_int_from_buffer(&data, &ipv4_addr_ptr)

//                        return ipv4_addr_ptr;
//                    log("Find an A-type record, ip addr is ");
//                    log(ipv4_addr_ptr);
//                    log();
                    break;

                case 0x05:
                    // CNAME
                    // TODO
                default:
                    // TODO

                    break;
            }
        }
        delete[] data;
    } else {
        // iteratively ...
        // TODO
    }

    delete send_buffer;
    delete rec_buffer;

    return &ipv4_addr_ptr;
}

/*
 * IPV6 2 threads
 * */

static std::string authority;
static pthread_t thread[2];
static pthread_mutex_t mut;
int version_done = 0;
bool ipv6_fail = false;
bool ipv4_fail = false;
int ipv4 = 0;
int ipv6[4];

char *get_name(char *buffer, char **start) {
    char *name = (char *) malloc(sizeof(char) * 64);
    int len = 0;
    while (true) {
        unsigned char x = (*start)[len];
        if (x == 0) {
            name[len - 1] = 0;
            *start += len + 1;
            break;
        } else if (x >= 0xc0) {
            unsigned short _len = (x << 8) + (*start)[len + 1];
            _len -= 0xc000;
            char *_start = buffer + _len;
            std::string reference_name = get_name(buffer, &_start);
            memcpy(name + len, reference_name.c_str(), reference_name.length() + 1);
            *start += len + 2;
            break;
        } else {
            //TODO
            memcpy(name + len, (*start) + len + 1, x);
            name[len + x] = '.';
            len = len + x + 1;
        }
    }
    return name;
}

RR *get_rr(char *buffer, char **start_ptr) {
    int type;
    int klass;
    int ttl;
    int data_length;
    char *data;
    std::string name;
    RR *rr = (RR *) malloc(sizeof(RR));
    char *_name = get_name(buffer, start_ptr);
    rr->name = new std::string(_name);
    char *r_b = *start_ptr;
    GET_TYPE
    rr->type = (TYPE) type;
    GET_KLASS
    rr->klass = (CLASS) klass;
    GET_TTL
    rr->ttl = ttl;
    GET_DATA_LENGTH
    rr->rdlength = data_length;
    data = (char *) malloc(sizeof(char) * data_length);
    memcpy(data, r_b, data_length);
    show_buffer1(data, data_length);
    rr->data = data;
    *start_ptr = r_b;
    if (type == CNAME) {
        _name = get_name(buffer, start_ptr);
        rr->cname = new std::string(_name);
    }
    return rr;
}

int dns_look_up1(std::string auth, bool ip_version) {
    using namespace std;
    int s;
    struct sockaddr_in addr;
    int addr_len = sizeof(struct sockaddr_in);

    char *send_buffer = new char[BUFFER_SIZE];
    char *rec_buffer = new char[BUFFER_SIZE];

//    if (ip_version)
//        log("IPV4 DNS\n");
//    else
//        log("IPV6 DNS\n");
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        log("udp socket error");
        return -1;
    }
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DNS_PORT);
    addr.sin_addr.s_addr = inet_addr(DNS_SERVER_ADDRESS);

    bzero(rec_buffer, sizeof(rec_buffer));

    send_size = 0;

    char _byte;
    short _word;
    char *s_b = send_buffer;

    /*
     * Part 1
     * Head Sector
     * */
    // BYTE 1 2
    // transaction id
    int tmp_id = ip_version ? random_transaction_id + 1 : random_transaction_id + 16;
    _word = (short) tmp_id;
    PUT_2

    // BYTE 3
    // Flag 1st half
    // 0 0000 0 0 1
    _byte = (char) 0x01;
    PUT_1

    // BYTE 4
    // Flag 2nd half
    _byte = (char) 0x00;
    PUT_1

    // BYTE 5 6
    // Question Numbers
    _word = (short) 0x01;
    PUT_2

    // BYTE 7 8
    // Answer Numbers
    _word = (short) 0000;
    PUT_2

    // BYTE 9 10
    // Authority Answer Numbers
    _word = (short) 0x00;
    PUT_2

    // BYTE 11 12
    // Additional Answer Numbers
    _word = (short) 0x00;
    PUT_2

    /*
     * Part 2
     * Questions
     * */

    int start_find = -1;
    int tmp_find;
    string section = auth;
    // QNAME
    // split with "."
    while ((tmp_find = auth.find('.', start_find + 1)) != -1) {
        section = auth.substr(start_find + 1, tmp_find - start_find - 1);
        int tmp_len = section.length();
        if (tmp_len > 0x100) {
            return 0;
        }
        _byte = (char) tmp_len;
        PUT_1
        PUT_S
        start_find = tmp_find;
    }

    section = auth.substr(start_find + 1, auth.length() - start_find - 1);
    _byte = section.length();
    PUT_1
    PUT_S

    _byte = (char) 0x00;
    PUT_1

    // QTYPE
    // A type 0x0001
    _word = ip_version ? (short) 0x0001 : (short) 0x001c; // REVISED HERE
    PUT_2

    // QCLASS
    // InterNet(IN) 0x0001
    _word = (short) 0x0001;
    PUT_2

//    show_buffer(send_buffer, send_size);
    /*
     * Send Query Message to DNS Server
     * */
    sendto(s, send_buffer, send_size, 0, (struct sockaddr *) &addr, addr_len);

    /*
     * Receive Response Message from DNS Server
     * */
    len = recvfrom(s, rec_buffer, BUFFER_SIZE, 0, (struct sockaddr *) &addr, (socklen_t *) &addr_len);
    close(s);
    if (len <= 0) {
        return -1;
    }
//    show_buffer1(rec_buffer, len);

//    if (DNS_RECUR_SUP) {
//        log("Recursive query not supported.\n");
//        return -1;
//    }
    if (DNS_ERROR) {
        log("DNS Response Code Error.\n");
        return -1;
    }
    // Check Transaction ID & Response Code
    if (GET_WORD(0) == (short) tmp_id) {
        log("Transaction id ");
        log_hex(tmp_id);
        log(" Matched\n");
    } else {
        log("Query id is ");
        log_hex((short) tmp_id);
        log(" , While Response id is ");
        log_hex(*(short *) rec_buffer);
        log();
        return -1;
    }
//    log();
    int questions;
    int answers;
    int authorities;
    int additional;
    questions = GET_QUESTIONS;
    answers = GET_ANSWERS;
    authorities = GET_AUTHs;
    additional = GET_ADDS;
    log("Question : ");
    log(questions);
    log(" Answers : ");
    log(answers);
    log(" Authority RRs : ");
    log(authorities);
    log(" Additional RRs : ");
    log(additional);
    log();

    // for example
    // www.zhihu.com 's length is 3+1+5+1+3   = 13B
    // name: 3www5zhihu3com0    1+3+1+5+1+3+1 = 13B+2B
    int name_len = auth.length() + 2;

    char *r_b = rec_buffer + name_len + 12 + 4;
    RR *results[answers];
    for (int j = 0; j < answers; j++) {
        RR *_rr = get_rr(rec_buffer, &r_b);
        results[j] = _rr;
        RR rr = *_rr;
        switch (rr.type) {
            case A:
                if (ip_version) {
                    pthread_mutex_lock(&mut);
                    if (version_done == 0) {
                        ipv4 = *((int *) (rr.data));
                        log(ipv4);
                        version_done = 4;
                    }
                    pthread_mutex_unlock(&mut);
                }
                break;
            case AAAA:
                if (!ip_version) {
                    pthread_mutex_lock(&mut);
                    if (version_done == 0) {
                        memcpy(ipv6, rr.data, 16);
                        version_done = 6;
                        log_ip1((int *)rr.data);
                        return -1;
                    }
//                    memcpy(ip_addr, rr->data, sizeof(char)*16);
                    pthread_mutex_unlock(&mut);
                }
                break;
            case CNAME:
                break;
        }
        if (version_done != 0)
            break;

    }
    if (version_done != 0) {
        return 0;
    }
    // recursively consult
    if (answers <= 0)
        return -1;
    log("DNS consult: " + *results[answers-1]->cname + "\n");
    if (dns_look_up1(*results[answers - 1]->cname, ip_version) != 0) {
        log("DNS error.\n");
        return -1;
    }
    return 0;
}

void *thread_v4(void *ptr) {
    if (dns_look_up1(authority, true) < 0)
        ipv4_fail = true;
    pthread_exit(NULL);
}

void *thread_v6(void *ptr) {
    if (dns_look_up1(authority, false) < 0)
        ipv6_fail = true;
    pthread_exit(NULL);
}

void thread_create(void) {
    int tmp;
    memset(&thread, 0, sizeof(thread));
    version_done = 0;
    if ((tmp = pthread_create(&thread[0], NULL, thread_v4, NULL)) != 0) {
        log("IPV4 thread creation failed.\n");
    }
    if ((tmp = pthread_create(&thread[1], NULL, thread_v6, NULL)) != 0) {
        log("IPV6 thread creation failed.\n");
    }
}

void thread_wait(void) {
    while (true) {
        if (version_done != 0) {
            return;
        }
        if (ipv6_fail & ipv4_fail)
            exit(0);
    }
}

/*
 * this function create 2 threads to generate DNS request simultaneously
 * when getting an address, it sets a mutex to be the ip version
 */
int *dns_look_up_v6(std::string auth) {
    using namespace std;
    authority = auth;
    pthread_mutex_init(&mut, NULL);
    thread_create();
    thread_wait();
    int *ret = (int *) malloc(sizeof(int) * 5);
    if (version_done == 4) {
        ret[0] = 4;
        ret[1] = ipv4;
        log("IPV4 addr = ");
        log(ipv4);
        log();
    } else if (version_done == 6) {
        ret[0] = 6;
        memcpy(ret + 1, ipv6, sizeof(int) * 4);
    } else
        return 0;
    return ret;
}

// unsed example code
//    len = read(STDIN_FILENO, send_buffer, sizeof(buffer));


//        int str_len = get_a_name_length(&r_b);
//        GET_TYPE
//        GET_KLASS
//        GET_TTL
//        GET_DATA_LENGTH
//        memcpy(data, r_b, data_length);
//        r_b += data_length;
//
//        // According to types of the answer
//        // Get Response Data
//        if (ipv4 && type == 0x01) {
//
//        } else if (ipv6 && type == 0x1c) {
//
//        }