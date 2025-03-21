#include "file_transfer.h"

void Client_Handle(int newfd, struct file_transfer *ft);

int main(int argc, char *argv[])
{
	char buf[BUFSIZ] = {};
	int fd, newfd;
	struct config conf;
	struct file_transfer *ft = (struct file_transfer *)buf;

	/*根据config初始化环境*/
	FT_InitConfig(&conf);

	/*准备套接字*/
	fd = SocketInit(conf.ip, conf.port, true);

	/*循环处理客户端请求*/
	while(1){
		/*接收来自客户端的连接*/
		newfd = accept(fd, NULL, NULL);
		Client_Handle(newfd, ft);
		close(newfd);
	}

	close(fd);

	return 0;
}

void Client_Handle(int newfd, struct file_transfer *ft){
	char *err_str = "Invalid option!";
	/*接收客户端数据*/
	while(Read(newfd, ft, SIZE_FT_TYPE) > 0){
		switch(ft->type){
		case TYPE_GET://处理获取文件的请求
			FT_GetFileHandler(newfd, ft);
			break;
		case TYPE_PUT://处理上传文件的请求
			FT_PutFileHandler(newfd, ft);
			break;
		case TYPE_LIST://处理获取文件列表的请求
			FT_FileListHandler(newfd, ft);
			break;
		case TYPE_SYNC://同步文件信息
			FT_SyncHandler(newfd, ft);
			break;
		default:
			Write(newfd, err_str, strlen(err_str));
		}
	}
}
