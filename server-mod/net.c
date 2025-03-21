#include "net.h"

int SocketInit(char *str_ip, char *str_port, bool server){
	int fd;
	Addr_in addr;

	/*创建套接字*/
	if( (fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0)
		ErrExit("socket");

	/*设置通信结构体*/
	bzero(&addr, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_port = htons( atoi(str_port) );
	if (inet_aton(str_ip, &addr.sin_addr) == 0) {
		fprintf(stderr, "Invalid address\n");
		exit(EXIT_FAILURE);
	}

	if(server){ //判断是否为服务端
		/*地址快速重用*/
		int b_reuse = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof(int) );

		/*绑定地址*/
		if( bind(fd, (Addr *)&addr, sizeof(addr) ) )
			ErrExit("bind");

		/*设定为监听模式*/
		if( listen(fd, BACKLOG) )
			ErrExit("listen");

	} else  /*如果是客户端就发起连接请求*/
		if( connect(fd, (Addr *)&addr, sizeof(addr) ) )
			ErrExit("connect");

	return fd;
}

ssize_t Read(int fd, void *buf, size_t len) {
	ssize_t ret;
	do {
		ret = read(fd, buf, len);
	} while(ret < 0 && errno == EINTR);
	if(ret < 0)
		ErrExit("read");
	return ret;
}

ssize_t Write(int fd, const void *buf, size_t len) {
	ssize_t ret;
	do {
		ret = write(fd, buf, len);
	} while(ret < 0 && errno == EINTR);
	if(ret < 0)
		ErrExit("write");
	return ret;
}

