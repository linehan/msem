#ifndef _SOCKET_H
#define _SOCKET_H
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * Protocol options
 */
#define PROTO_IP   IPPROTO_IP    /* dummy for IP */
#define PROTO_IPV4 IPPROTO_IPV4  /* internet protocol version 4 */
#define PROTO_IPV6 IPPROTO_IPV6  /* internet protocol version 6 */
#define PROTO_TCP  IPPROTO_TCP   /* transmission control protocol */
#define PROTO_UDP  IPPROTO_UDP   /* user datagram protocol */
#define PROTO_ICMP IPPROTO_ICMP  /* internet control and messaging protocol */

/*
 * Internet addresses
 */
#define ADDR_ANY        INADDR_ANY
#define ADDR_LOOPBACK   INADDR_LOOPBACK
#define ADDR_BROADCAST  INADDR_BROADCAST

/*
 * Ports < IPPORT_RESERVED are for privleged
 * processes (e.g. the kernel).
 * 
 * Ports > IPPORT_USERRESERVED are usually
 * for servers.
 */
#define IPPORT_RESERVED         1024
#define IPPORT_USERRESERVED     5000
#define IPPORT_MAXIMUM          65535
#define IPPORT_STRLEN           6 

/* Length of an IPv4 string */
#define IPV4_ADDR_STRLEN 16

/* Length of an IPv6 string */
#define IPV6_ADDR_STRLEN 46

/* Maximum size of the accept queue. */
#define BACKLOG_MAX 64


#define PORT_DNS 53


#define IP4_MIN_PACKET 20
#define IP4_MIN_HEADER 20
#define IP4_MAX_HEADER 60

#define PACKETSIZE_ICMP    32 
#define PACKETSIZE_ICMP_SM 64 
#define PACKETSIZE_MAX  1024 


struct sockspec_t {
        int domain;
        int type;
        int protocol;
};


enum socktype { 
        ICMP,
        TCP, 
        UDP, 
        RAW 
};

#define addr_dest(sock) sock->a_dest.sin_addr
#define addr_orig(sock) sock->a_orig.sin_addr


struct sock_t {
        /* Options and configuration info */
        struct sockspec_t t;
        /* Protocol-specific header */
        char header[IP4_MAX_HEADER];
        /* Packet payload */
        char data[PACKETSIZE_MAX-IP4_MAX_HEADER];
        /* Packet payload. */
        char packet[PACKETSIZE_MAX];
        /* Originating address */
        struct sockaddr_in a_orig;
        /* Destination address */
        struct sockaddr_in a_dest;
        /* Length of originating address */
        socklen_t l_orig;
        /* Length of destination address */
        socklen_t l_dest;
        /* Socket file descriptor */
        int socket;
};



static inline char *ip_str(struct sockaddr_in *addr)
{
        return inet_ntoa(addr->sin_addr);
}


extern void sock_setup_server(struct sock_t *sock, int port, enum socktype tag);
extern void sock_setup_client(struct sock_t *sock, const char *dst, enum socktype tag);

extern int str_to_addr(int family, const char *addr, struct in_addr *dst);
extern char *addr_to_str(int family, struct in_addr *addr);

#define A_LOCAL  0
#define A_REMOTE 1

extern char *sock_address(struct sock_t *sock, int option);

extern void sock_open(struct sock_t *sock);
extern void sock_bind(struct sock_t *sock);
extern void sock_listen(struct sock_t *sock, int backlog);
extern int sock_accept(struct sock_t *sock);
extern void sock_connect(struct sock_t *sock);

extern void sock_block(struct sock_t *sock, bool setting);


extern int sock_disconnect(struct sock_t *sock);
extern bool sock_setup_client_hostname(struct sock_t *sock, const char *URI);

extern int sock_option(struct sock_t *sock, int option, int value);

extern void sock_memset_packet(struct sock_t *sock, char byte);
extern void sock_memset_header(struct sock_t *sock, char byte);

extern int sock_send(struct sock_t *sock, void *packet, int packetsize);
extern int sock_scan(struct sock_t *sock, void *packet, int packetsize);

#endif
