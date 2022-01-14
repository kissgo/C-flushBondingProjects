﻿#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> //  /usr/include/i386-linux-gnu/sys
#include <sys/stat.h>  // /usr/include/sys
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <linux/input.h>   // /usr/include/linux


//#include "../../include/qisr.h"
#include "../../include/qisr.h"
#include "../../include/msp_cmn.h"
#include "../../include/msp_errors.h"

#define SAMPLE_RATE_16K     (16000)
#define SAMPLE_RATE_8K      (8000)
#define MAX_GRAMMARID_LEN   (32)
#define MAX_PARAMS_LEN      (1024)

/***********电脑Ubuntu和开发板的IP及端口号******************/
#define  IP_ADDR "192.168.0.200"  //电脑Ubuntu的IP
#define  IP_ADDR2 "192.168.0.100"  //开发板的IP
#define  PORT_NUM  50000	//电脑的端口号
#define  PORT_NUM2  50001	//开发板的端口号
int udpsock;
/***********************************************************/

const char * ASR_RES_PATH        = "fo|res/asr/common.jet"; //离线语法识别资源路径
const char * GRM_BUILD_PATH      = "res/asr/GrmBuilld"; //构建离线语法识别网络生成数据保存路径
const char * GRM_FILE            = "call.bnf"; //构建离线识别语法网络所用的语法文件
const char * LEX_NAME            = "contact"; //更新离线识别语法的contact槽（语法文件为此示例中使用的call.bnf）

typedef struct _UserData {
	int     build_fini; //标识语法构建是否完成
	int     update_fini; //标识更新词典是否完成
	int     errcode; //记录语法构建或更新词典回调错误码
	char    grammar_id[MAX_GRAMMARID_LEN]; //保存语法构建返回的语法ID
}UserData;


const char *get_audio_file(void); //选择进行离线语法识别的语音文件
int build_grammar(UserData *udata); //构建离线识别语法网络
int update_lexicon(UserData *udata); //更新离线识别语法词典
int run_asr(UserData *udata); //进行离线语法识别

const char* get_audio_file(void)
{
	char key = 0;
	while(key != 27) //按Esc则退出
	{
		printf("请选择音频文件：\n");
		printf("1.打电话给丁伟\n");
		printf("2.打电话给黄辣椒\n");
		key = getchar();
		getchar();
		switch(key)
		{
		case '1':
			printf("\n1.打电话给丁伟\n");
			return "wav/ddhgdw.pcm";
		case '2':
			printf("\n2.打电话给黄辣椒\n");
			return "wav/ddhghlj.pcm";
		default:
			continue;
		}
	}
	exit(0);
	return NULL;
}

int build_grm_cb(int ecode, const char *info, void *udata)
{
	UserData *grm_data = (UserData *)udata;

	if (NULL != grm_data) {
		grm_data->build_fini = 1;
		grm_data->errcode = ecode;
	}

	if (MSP_SUCCESS == ecode && NULL != info) {
		printf("构建语法成功！ 语法ID:%s\n", info);
		if (NULL != grm_data)
			snprintf(grm_data->grammar_id, MAX_GRAMMARID_LEN - 1, info);
	}
	else
		printf("构建语法失败！%d\n", ecode);

	return 0;
}

int build_grammar(UserData *udata)
{
	FILE *grm_file                           = NULL;
	char *grm_content                        = NULL;
	unsigned int grm_cnt_len                 = 0;
	char grm_build_params[MAX_PARAMS_LEN]    = {NULL};
	int ret                                  = 0;

	grm_file = fopen(GRM_FILE, "rb");	
	if(NULL == grm_file) {
		printf("打开\"%s\"文件失败！[%s]\n", GRM_FILE, strerror(errno));
		return -1; 
	}

	fseek(grm_file, 0, SEEK_END);
	grm_cnt_len = ftell(grm_file);
	fseek(grm_file, 0, SEEK_SET);

	grm_content = (char *)malloc(grm_cnt_len + 1);
	if (NULL == grm_content)
	{
		printf("内存分配失败!\n");
		fclose(grm_file);
		grm_file = NULL;
		return -1;
	}
	fread((void*)grm_content, 1, grm_cnt_len, grm_file);
	grm_content[grm_cnt_len] = '\0';
	fclose(grm_file);
	grm_file = NULL;

	snprintf(grm_build_params, MAX_PARAMS_LEN - 1, 
		"engine_type = local, \
		asr_res_path = %s, sample_rate = %d, \
		grm_build_path = %s, ",
		ASR_RES_PATH,
		SAMPLE_RATE_16K,
		GRM_BUILD_PATH
		);
	ret = QISRBuildGrammar("bnf", grm_content, grm_cnt_len, grm_build_params, build_grm_cb, udata);

	free(grm_content);
	grm_content = NULL;

	return ret;
}

int update_lex_cb(int ecode, const char *info, void *udata)
{
	UserData *lex_data = (UserData *)udata;

	if (NULL != lex_data) {
		lex_data->update_fini = 1;
		lex_data->errcode = ecode;
	}

	if (MSP_SUCCESS == ecode)
		printf("更新词典成功！\n");
	else
		printf("更新词典失败！%d\n", ecode);

	return 0;
}

int update_lexicon(UserData *udata)
{
	const char *lex_content                   = "丁伟\n黄辣椒";
	unsigned int lex_cnt_len                  = strlen(lex_content);
	char update_lex_params[MAX_PARAMS_LEN]    = {NULL}; 

	snprintf(update_lex_params, MAX_PARAMS_LEN - 1, 
		"engine_type = local, text_encoding = UTF-8, \
		asr_res_path = %s, sample_rate = %d, \
		grm_build_path = %s, grammar_list = %s, ",
		ASR_RES_PATH,
		SAMPLE_RATE_16K,
		GRM_BUILD_PATH,
		udata->grammar_id);
	return QISRUpdateLexicon(LEX_NAME, lex_content, lex_cnt_len, update_lex_params, update_lex_cb, udata);
}

int run_asr(UserData *udata)
{
	char asr_params[MAX_PARAMS_LEN]    = {NULL};
	const char *rec_rslt               = NULL;
	const char *session_id             = NULL;
	const char *asr_audiof             = NULL;
	FILE *f_pcm                        = NULL;
	char *pcm_data                     = NULL;
	long pcm_count                     = 0;
	long pcm_size                      = 0;
	int last_audio                     = 0;
	int aud_stat                       = MSP_AUDIO_SAMPLE_CONTINUE;
	int ep_status                      = MSP_EP_LOOKING_FOR_SPEECH;
	int rec_status                     = MSP_REC_STATUS_INCOMPLETE;
	int rss_status                     = MSP_REC_STATUS_INCOMPLETE;
	int errcode                        = -1;

	/*****************/
	struct sockaddr_in saddr;
	int addrsize=sizeof(struct sockaddr_in);
	
	//准备开发板的IP和端口号
	bzero(&saddr,addrsize);
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(PORT_NUM2);//开发板的端口号
	saddr.sin_addr.s_addr=inet_addr(IP_ADDR2);//开发板的IP地址	
	/*************************/
	//asr_audiof = get_audio_file();
	f_pcm = fopen("1.wav", "rb");
	if (NULL == f_pcm) {
		printf("打开\"%s\"失败！[%s]\n", f_pcm, strerror(errno));
		goto run_error;
	}
	printf("打开成功\n");
	fseek(f_pcm, 0, SEEK_END);
	pcm_size = ftell(f_pcm);
	fseek(f_pcm, 0, SEEK_SET);
	pcm_data = (char *)malloc(pcm_size);
	if (NULL == pcm_data)
		goto run_error;
	fread((void *)pcm_data, pcm_size, 1, f_pcm);
	fclose(f_pcm);
	f_pcm = NULL;

	//离线语法识别参数设置
	snprintf(asr_params, MAX_PARAMS_LEN - 1, 
		"engine_type = local, \
		asr_res_path = %s, sample_rate = %d, \
		grm_build_path = %s, local_grammar = %s, \
		result_type = xml, result_encoding = UTF-8, ",
		ASR_RES_PATH,
		SAMPLE_RATE_16K,
		GRM_BUILD_PATH,
		udata->grammar_id
		);
	session_id = QISRSessionBegin(NULL, asr_params, &errcode);
	if (NULL == session_id)
		goto run_error;
	printf("开始识别...\n");

	while (1) {
		unsigned int len = 6400;

		if (pcm_size < 12800) {
			len = pcm_size;
			last_audio = 1;
		}

		aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;

		if (0 == pcm_count)
			aud_stat = MSP_AUDIO_SAMPLE_FIRST;

		if (len <= 0)
			break;

		printf(">");
		fflush(stdout);
		errcode = QISRAudioWrite(session_id, (const void *)&pcm_data[pcm_count], len, aud_stat, &ep_status, &rec_status);
		if (MSP_SUCCESS != errcode)
			goto run_error;

		pcm_count += (long)len;
		pcm_size -= (long)len;

		//检测到音频结束
		if (MSP_EP_AFTER_SPEECH == ep_status)
			break;

		usleep(150 * 1000); //模拟人说话时间间隙
	}
	//主动点击音频结束
	QISRAudioWrite(session_id, (const void *)NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_status, &rec_status);

	free(pcm_data);
	pcm_data = NULL;

	//获取识别结果
	while (MSP_REC_STATUS_COMPLETE != rss_status && MSP_SUCCESS == errcode) {
		rec_rslt = QISRGetResult(session_id, &rss_status, 0, &errcode);
		usleep(150 * 1000);
	}
	printf("\n识别结束：\n");
	printf("=============================================================\n");
	
	/***********
	根据识别到结果，发送对应的设定好的字符串给开发板
	
	*************/
	if (NULL != rec_rslt)
	{
		if(strstr(rec_rslt,"相册") != NULL)
		{
			sendto(udpsock,"xc",4,0,(struct sockaddr *)&saddr,addrsize);			
		}
		else if(strstr(rec_rslt,"视频") != NULL)
		{
			sendto(udpsock,"sp",5,0,(struct sockaddr *)&saddr,addrsize);			
		}
		else if(strstr(rec_rslt,"菜单") != NULL)
		{
			sendto(udpsock,"fh",5,0,(struct sockaddr *)&saddr,addrsize);			
		}
		else if(strstr(rec_rslt,"快进") != NULL)
		{
			sendto(udpsock,"kj",5,0,(struct sockaddr *)&saddr,addrsize);			
		}
		else if(strstr(rec_rslt,"下一首") != NULL)
		{
			sendto(udpsock,"kt",5,0,(struct sockaddr *)&saddr,addrsize);			
		}
		else if(strstr(rec_rslt,"暂停") != NULL)
		{
			sendto(udpsock,"zt",5,0,(struct sockaddr *)&saddr,addrsize);			
		}
		else if(strstr(rec_rslt,"播放") != NULL)
		{
			sendto(udpsock,"bf",5,0,(struct sockaddr *)&saddr,addrsize);			
		}
		else{
			sendto(udpsock,"*****",5,0,(struct sockaddr *)&saddr,addrsize);
		}
		printf("%s\n", rec_rslt);
	}	
	else
		printf("没有识别结果！\n");
	printf("=============================================================\n");

	goto run_exit;

run_error:
	if (NULL != pcm_data) {
		free(pcm_data);
		pcm_data = NULL;
	}
	if (NULL != f_pcm) {
		fclose(f_pcm);
		f_pcm = NULL;
	}
run_exit:
	QISRSessionEnd(session_id, NULL);
	return errcode;
}

//定义一个ipv4地址结构体变量
struct sockaddr_in baddr;
//是否识别音频文件的标志
int shibie=0;
//封装初始化udp通信的函数
int udp_init()
{
	int ret; 
	int addrsize=sizeof(struct sockaddr_in);
	bzero(&baddr,addrsize);
	baddr.sin_family=AF_INET;
	baddr.sin_port=htons(PORT_NUM);//端口号，同一台电脑上不同的程序需要使用不同的端口号
	baddr.sin_addr.s_addr=inet_addr(IP_ADDR);//电脑UBUNTU的IP
	
	// 创建udp套接字
	udpsock=socket(AF_INET,SOCK_DGRAM,0);
	if(udpsock==-1)
	{
		perror("create udp sock failed!\n");
		return -1;
	}
	// 取消绑定限制
	int sinsize = 1;
	setsockopt(udpsock, SOL_SOCKET, SO_REUSEADDR, &sinsize, sizeof(int)); 
	ret=bind(udpsock,(struct sockaddr *)&baddr,addrsize);
	if(ret==-1)
	{
		perror("bind failed!\n");
		return -1;
	} 
	return 0;
}
//接收开发板发送过来的音频文件的线程
void *get_wav(void *arg)
{
	char rbuf[1024] = {0};//定义一个数组，用来接收网络收到的数据
	int addrsize=sizeof(struct sockaddr_in);
	int ret ;
	int fd;
	int flag;
	
	
	while(1)
	{
		flag=1;//打开音频文件的标志
		//这个循环是接收一次完整的音频文件的过程
		while(1)
		{
			//接收开发板发送过来的音频文件
			ret = recvfrom(udpsock,rbuf,1024,0,(struct sockaddr *)&baddr,&addrsize);
			if(flag)
			{
				//打开本地的音频文件，如果文件不存在就创建
				fd = open("1.wav",O_RDWR | O_CREAT);
				if(fd<0)
				{
					perror("open fail");
					return -1;
				}
				else
				{
					flag=0;
				}
			}		
			//将接收到的音频数据写入到本地要识别的音频文件
			write(fd,rbuf,ret);
			if(ret < 1024)
			{
				close(fd);
				break;
			}	
		}
		//音频文件接收结束，就可以识别了
		printf("jieshoochenggong");
		shibie = 1;
	}
	
	
}

int main(int argc, char* argv[])
{
	const char *login_config    = "appid = a878c86f"; //登录参数
	UserData asr_data; 
	int ret                     = 0 ;
	pthread_t thread;
	
	ret = MSPLogin(NULL, NULL, login_config); //第一个参数为用户名，第二个参数为密码，传NULL即可，第三个参数是登录参数
	if (MSP_SUCCESS != ret) {
		printf("登录失败：%d\n", ret);
		goto exit;
	}
	memset(&asr_data, 0, sizeof(UserData));
	printf("构建离线识别语法网络...\n");
	ret = build_grammar(&asr_data);  //第一次使用某语法进行识别，需要先构建语法网络，获取语法ID，之后使用此语法进行识别，无需再次构建
	if (MSP_SUCCESS != ret) {
		printf("构建语法调用失败！\n");
		goto exit;
	}
	while (1 != asr_data.build_fini)
		usleep(300 * 1000);
	if (MSP_SUCCESS != asr_data.errcode)
		goto exit;
	printf("离线识别语法网络构建完成，开始识别...\n");	
	
	//网络初始化
	udp_init();
	//启动接收音频文件的线程
	pthread_create(&thread,NULL,get_wav,NULL);
	
	while(1)
	{
		//等待接收音频文件完毕后开始识别
		if(shibie)
		{
			ret = run_asr(&asr_data);
			if (MSP_SUCCESS != ret) {
				printf("离线语法识别出错: %d \n", ret);
			
			}	
			shibie = 0;
		}
			
	}
	



exit:
	MSPLogout();
	printf("请按任意键退出...\n");
	getchar();
	return 0;
}

