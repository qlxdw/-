#include "file_transfer.h"

/*私有函数*/
/*********************************/
/*创建文件,并写入空内容*/
static int W_zero(char *f_name, long long f_size){
	int count, ret;
	char buf[BUFSIZ] = {};
	int f_fd = open(f_name, O_WRONLY|O_CREAT|O_TRUNC, 0640);
	if(f_fd < 0){
		perror("open");
		return f_fd;
	}
	/*占领磁盘空间(内核一次最多能写入BUFSIZ)*/
	while(f_size > 0){
		if(f_size > BUFSIZ) count = BUFSIZ;
		else count = f_size;
		ret = Write(f_fd, buf, count);
		if(ret < 0)
			return ret;
		f_size -= ret;
	}
	close(f_fd);
	return 0;
}

/*接收文件*/
void W_body(int sockfd,char *f_name, long long f_size){
	char buf[BUFSIZ] = {};
	int f_fd, ret, count;
	if( (f_fd = open(f_name, O_WRONLY) ) < 0)
		ErrExit("open");
	while(f_size > 0){
		if(f_size > BUFSIZ) count = BUFSIZ;
		else count = f_size;
		ret = Read(sockfd, buf, count);
		Write(f_fd, buf, ret);
		f_size -= ret;
	}
	close(f_fd);
}

/*发送文件*/
void R_body(int sockfd,char *f_name, long long f_size){
	char buf[BUFSIZ] = {};
	int f_fd, count, ret;
	/*打开文件*/
	if( (f_fd = open(f_name, O_RDONLY) ) < 0)
		return;
	while(f_size > 0){
		if(f_size > BUFSIZ) count = BUFSIZ;
		else count = f_size;
		ret = Read(f_fd, buf, count);
		Write(sockfd, buf, ret);
		f_size -= ret;
	}
	close(f_fd);
}

/**客户端函数**/
/*********************************/
/*初始化环境*/
void FT_InitConfig(struct config *conf){
	size_t len;
	FILE *fp = fopen(CONFIG_FILE, "r");
	if(fp == NULL)
		ErrExit("fopen");
	/*设置IP地址*/
	fgets(conf->ip, SIZE_IP_STR, fp);
	len = strlen(conf->ip);
	if(conf->ip[len-1] == '\n')
		conf->ip[len-1] = '\0';
	/*设置端口号*/
	fgets(conf->port, SIZE_IP_STR, fp);
	len = strlen(conf->port);
	if(conf->port[len-1] == '\n')
		conf->port[len-1] = '\0';
#ifdef DEBUG
	printf("[%s:%d] conf->ip=%s\n", __FUNCTION__, __LINE__,
			conf->ip);
	printf("[%s:%d] conf->port=%s\n", __FUNCTION__, __LINE__, 
			conf->port);
#endif
	fclose(fp);
}
/*获取文件*/
void FT_GetFile(int sockfd, char *f_name, struct file_transfer *ft){
	printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	long long f_size;
	ft->type = TYPE_GET;
	/*发送请求信息*/
	Write(sockfd, ft, SIZE_FT_TYPE);
	ft->len = strlen(f_name);
	Write(sockfd, &ft->len, 1);
	Write(sockfd, f_name, ft->len);
	/*接收请求结果*/
	Read(sockfd, ft, SIZE_FT_TYPE);
	if(ft->type == TYPE_OK){
		Read(sockfd, &f_size, SIZE_FT_F_SIZE);
		/*创建文件,并写入空内容*/
		W_zero(f_name, f_size);
		/*写入文件内容*/
		W_body(sockfd, f_name, f_size);
	}else{
		/*如果接收文件失败，打印错误信息*/
		Read(sockfd, &ft->len, 1);
		Read(sockfd, ft->msg, ft->len);
		ft->msg[ft->len] = '\0';
		printf("get file [%s] failed [%s]\n", f_name, ft->msg);
	}
}
/*上传文件*/
void FT_PutFile(int sockfd, char *f_name, struct file_transfer *ft){
	printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	/*打开文件*/
	int f_fd = open(f_name, O_RDONLY);
	if(f_fd < 0)
		ErrExit("open");
	/*获取文件属性*/
	struct stat st;
	if( fstat(f_fd, &st) )
		ErrExit("stat");
	/*检查文件类型:只能上传普通文件*/
	if(!(st.st_mode & S_IFREG))
		return;
	/*获取文件大小*/
	long long f_size = (long long)st.st_size;
	/*设置自定义协议*/
	ft->type = TYPE_PUT; //消息类型
	Write(sockfd, ft, SIZE_FT_TYPE); //发送消息类型
	Write(sockfd, &f_size, SIZE_FT_F_SIZE); //发送文件大小
	ft->len = strlen(f_name);
	Write(sockfd, &ft->len, 1);
	Write(sockfd, f_name, ft->len);  //发送文件名字
	/*等待确认*/
	if( !Read(sockfd, ft, SIZE_FT_TYPE) )
		return;
	if(ft->type == TYPE_OK){  //发送文件
		R_body(sockfd, f_name, f_size);
	}else{                    //如果确认失败，打印错误信息
		Read(sockfd, &ft->len, 1);
		Read(sockfd, ft->msg, ft->len);
		ft->msg[ft->len] = '\0';
		printf("get file [%s] failed [%s]\n", f_name, ft->msg);
	}
	/*关闭文件*/
	close(f_fd);
}
/*获取文件列表*/
void FT_FileList(int sockfd, struct file_transfer *ft){
	printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	time_t mtime;
	ft->type = TYPE_LIST;
	Write(sockfd, &ft->type, SIZE_FT_TYPE);
	if( Read(sockfd, ft, SIZE_FT_TYPE) == 0)
		return;
	if(ft->type == TYPE_OK){
		while(1){
			Read(sockfd, &ft->len, 1);
			Read(sockfd, ft->f_name, ft->len);
			if(ft->len == 0)
				break;
			ft->f_name[ft->len] = '\0';
			printf("%-32s", ft->f_name);
			Read(sockfd, &mtime, sizeof(time_t) );
			printf("%s", ctime(&mtime) );
		}
	}else{
		printf("get list failed [%s]", ft->msg);
	}
}
/*同步文件信息*/
void FT_Sync(int sockfd, struct file_transfer *ft){
	printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	DIR *dir;
	struct dirent *p;
	/*打开目录*/
	if( (dir = opendir(".") ) == NULL)
		ErrExit("opendir");
	/*读取目录*/
	while( (p = readdir(dir) ) != NULL ){
		if(p->d_type == DT_REG && p->d_name[0] != '.'){
			printf("输入任意键，继续上传文件%s\n", p->d_name);
			FT_PutFile(sockfd, p->d_name, ft);
		}
	}
	closedir(dir);
}
/*********************************/

/**服务端函数**/
/*********************************/
/*处理获取文件的请求*/
void FT_GetFileHandler(int sockfd, struct file_transfer *ft){
	printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	Read(sockfd, &ft->len, 1); 
	Read(sockfd, ft->f_name, ft->len); 
	ft->f_name[ft->len] = '\0';
	/*获取文件属性*/
	struct stat st;
	if( stat(ft->f_name, &st) )
		ErrExit("stat");
	/*检查文件类型:只能下载普通文件*/
	char *errmsg = "只可以下载普通文件\n";
	if(!(st.st_mode & S_IFREG) || ft->f_name[0] == '.'){
		ft->type = TYPE_ERR;
		Write(sockfd, &ft->type, SIZE_FT_TYPE);
		ft->len = strlen(errmsg);
		Write(sockfd, &ft->len, 1);
		Write(sockfd, errmsg, ft->len);
		return;
	}
	ft->type = TYPE_OK;
	Write(sockfd, &ft->type, SIZE_FT_TYPE);
	long long f_size = (long long)st.st_size;
	Write(sockfd, &f_size, SIZE_FT_F_SIZE);
	/*发送文件内容*/
	R_body(sockfd, ft->f_name, f_size);
}
/*处理上传文件的请求*/
void FT_PutFileHandler(int sockfd, struct file_transfer *ft){
	printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	long long f_size;
	/*创建文件*/
	Read(sockfd, &f_size, SIZE_FT_F_SIZE);
	Read(sockfd, &ft->len, 1);
	Read(sockfd, ft->f_name, ft->len);
	ft->f_name[ft->len] = '\0';
	if(ft->f_name[0] == '.'){
		ft->type = TYPE_ERR;
		sprintf(ft->msg, "不可以上传隐藏文件\n");
		Write(sockfd, ft, SIZE_FT_TYPE+strlen(ft->msg)+1);
		return;
	}
	int f_fd = open(ft->f_name, O_WRONLY|O_CREAT|O_TRUNC, 0640);
	if(f_fd < 0) {
		ft->type = TYPE_ERR;
		Write(sockfd, &ft->type, SIZE_FT_TYPE);
		sprintf(ft->msg, "[创建打开失败][open:%s]\n", strerror(errno) );
		ft->len = strlen(ft->msg);
		Write(sockfd, &ft->len, 1);
		Write(sockfd, ft->msg, ft->len);
		printf("[send msg]%s\n", ft->msg);
		return;
	}
	/*占领磁盘空间(内核一次最多能写入BUFSIZ)*/
	if (W_zero(ft->f_name, f_size) < 0){
		ft->type = TYPE_ERR;
		Write(sockfd, &ft->type, SIZE_FT_TYPE);
		sprintf(ft->msg, "[磁盘空间不足][write:%s]\n", strerror(errno) );
		ft->len = strlen(ft->msg);
		Write(sockfd, &ft->len, 1);
		Write(sockfd, ft->msg, ft->len);
		printf("[send msg]%s\n", ft->msg);
		return;
	}

	/*发送确认信息*/
	ft->type = TYPE_OK;
	if( Write(sockfd, ft, 1) < 0)
		return;
	/*写入文件内容*/
	W_body(sockfd, ft->f_name, f_size);
	printf("[文件接收成功]文件名: %-32s文件大小：%lld\n", ft->f_name, f_size);
}
/*处理获取文件列表的请求*/
void FT_FileListHandler(int sockfd, struct file_transfer *ft){
	printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	DIR *dir;
	struct dirent *p;
	struct stat st;
	/*打开目录*/
	if( (dir = opendir(".") ) == NULL){
		ft->type = TYPE_ERR;
		sprintf(ft->msg, "[目录打开失败][opendir:%s]\n", strerror(errno) );
		Write(sockfd, ft, SIZE_FT_TYPE+strlen(ft->msg)+1);
		return;
	}
	/*读取目录*/
	ft->type = TYPE_OK;
	Write(sockfd, ft, SIZE_FT_TYPE);
	while( (p = readdir(dir) ) != NULL ){
		if(p->d_type == DT_REG && p->d_name[0] != '.'){
			ft->len = strlen(p->d_name);
			Write(sockfd, &ft->len, 1);
			Write(sockfd, p->d_name, ft->len);
			stat(p->d_name, &st);
			Write(sockfd, &st.st_mtime, sizeof(time_t) );
		}
	}
	ft->len = 0;
	Write(sockfd, &ft->len, 1);
	closedir(dir);
}
/*同步文件信息*/
void FT_SyncHandler(int sockfd, struct file_transfer *ft){
}

