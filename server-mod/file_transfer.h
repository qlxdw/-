#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

#include "net.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

#define DEBUG printf("==debug information==\n");

#define CONFIG_FILE ".config"
#define SIZE_IP_STR 17
#define SIZE_PORT_STR 6
#define SIZE_PATH_STR 255
#define SIZE_CLOUD_SPACE 1024*1024*1024   //云空间大小 单位:byte

#define SIZE_FT_TYPE 1 //文件类型大小
#define SIZE_FT_F_SIZE 8 //文件大小

/*自定义协议类型*/
enum {
	TYPE_ERR, TYPE_OK, TYPE_GET, TYPE_PUT, TYPE_LIST, TYPE_SYNC,
};

/*自定义协议结构体*/
struct file_transfer {
	uint8_t type;
	/*GNU C的扩展属性，0长度的数组不占用内存空间
	 * msg 有效时，其他字段无效
	 * */
	char f_size[8];//文件大小
	uint8_t len;//name长度
	char f_name[0];//文件名
	char msg[0];     
	char f_body[0];
};

/*环境信息*/
struct config{
	char ip[SIZE_IP_STR];
	char port[SIZE_PORT_STR];
};

/*文件链表*/
typedef struct node{
	time_t mtime;
	char name[NAME_MAX];
	struct node *next;
}node_t;

/**客户端函数**/
/*********************************/
/*初始化环境*/
void FT_InitConfig(struct config *conf);
/*获取文件*/
void FT_GetFile(int sockfd, char *f_name, struct file_transfer *ft);
/*上传文件*/
void FT_PutFile(int sockfd, char *f_name, struct file_transfer *ft);
/*获取文件列表*/
void FT_FileList(int sockfd, struct file_transfer *ft);
/*同步文件信息*/
void FT_Sync(int sockfd, struct file_transfer *ft);
/*********************************/

/**服务端函数**/
/*********************************/
/*处理获取文件的请求*/
void FT_GetFileHandler(int sockfd, struct file_transfer *ft);
/*处理上传文件的请求*/
void FT_PutFileHandler(int sockfd, struct file_transfer *ft);
/*处理获取文件列表的请求*/
void FT_FileListHandler(int sockfd, struct file_transfer *ft);
/*同步文件信息*/
void FT_SyncHandler(int sockfd, struct file_transfer *ft);

#endif
