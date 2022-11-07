struct addrinfo;

int getsock(const struct addrinfo *rp);
int getconn(int sock, const struct addrinfo *rp);
int connect_first(const struct addrinfo *rp);
int tcp_connect(const char *host, const char *port);

