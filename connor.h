#pragma once

struct addrinfo;

struct addrinfo *get_tcp_addrinfo(const char *host, const char *port);
int connect_first(const struct addrinfo *rp);
int tcp_connect(const char *host, const char *port);

