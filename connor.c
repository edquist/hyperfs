#define _POSIX_C_SOURCE 201112L

#include <stdio.h>       // printf
#include <stdlib.h>      // exit
#include <sys/types.h>
#include <sys/socket.h>  // socket, connect
#include <netdb.h>       // struct addrinfo
#include <unistd.h>      // close


// int socket(int domain, int type, int protocol);

// int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

// opening /dev/tcp/192.168.1.6/9080 ->
// socket(PF_INET, SOCK_STREAM, IPPROTO_TCP) = 4
// connect(4, {sa_family=AF_INET, sin_port=htons(9080),
//             sin_addr=inet_addr("192.168.1.6")}, 16) = 0


int getsock(const struct addrinfo *rp)
{
	return socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
}

int getconn(int sock, const struct addrinfo *rp)
{
	return connect(sock, rp->ai_addr, rp->ai_addrlen);
}

int connect_first(const struct addrinfo *rp)
{
	int sock;

	// follow list of addresses until socket+connect succeeds
	for (; rp; rp = rp->ai_next) {
		if ((sock = getsock(rp)) < 0)
			continue;

		if (getconn(sock, rp) == 0)
			break;  // Success

		close(sock);
	}

	return rp ? sock : -1;
}

int tcp_connect(const char *host, const char *port)
{

        struct addrinfo *result;
        int s;

	int sock;

	struct addrinfo hints = {
		.ai_family   = AF_INET,
		.ai_socktype = SOCK_STREAM,
		.ai_flags    = 0,  // AI_NUMERICHOST to avoid DNS lookup
		.ai_protocol = IPPROTO_TCP
	};

	s = getaddrinfo(host, port, &hints, &result);

	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(1);
	}

	sock = connect_first(result);

	if (sock == -1) {
		fprintf(stderr, "Could not connect\n");
		exit(1);
	}

	freeaddrinfo(result);

	return sock;
}

