#ifndef _NET_H_
#define _NET_H_

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#define BACKLOG 5

#define ErrExit(msg) do { \
	fprintf(stderr, "[%s:%d] ", \
			__FUNCTION__, __LINE__); \
	perror(msg); \
	exit(EXIT_FAILURE); } while(0)

typedef struct sockaddr Addr;
typedef struct sockaddr_in Addr_in;

void Argment(int argc, char *argv[]);
int SocketInit(char *addr, char *port, bool server);

ssize_t Read(int fd, void *buf, size_t len);
ssize_t Write(int fd, const void *buf, size_t len);

#endif
