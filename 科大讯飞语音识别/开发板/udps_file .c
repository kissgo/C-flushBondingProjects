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
#include <fcntl.h>
#include <unistd.h>//close、read、write
#include <string.h>//strlen
#include <linux/input.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>

//#include "a.h"


#define  IP_ADDR "192.168.0.100"  //自己的IP
#define  IP_ADDR2 "192.168.0.200"  //对方的IP
#define  PORT_NUM  50001	//自己的端口号
#define  PORT_NUM2  50000	//对方的端口号

int udpsock;

int fd;
int touch_fd;
int bmpfd ;
int *lcd ;
int x1=0,x2=0;//用来暂存x1,x2的值
int x_now=0,y_now=0,key=1;//实时坐标
int pi=0;//bmp指针
int flag=-1; //进入标志 1相册
int flag2=0;
int i=0;//钢琴黑白变量
int fifo_fd=0;//管道变量
char cmd1[256]={0};

int flag1=1;

unsigned char r;
unsigned char g;
unsigned char b;
unsigned char a;
unsigned int color =0;
unsigned int x=0,y=0;



	
	

char *pic[8]={//电子相册的八张照片
		"1.bmp",
		"2.bmp",
		"3.bmp",
		"4.bmp",
		"5.bmp",
		"6.bmp",
		"7.bmp",
		"0.bmp"	
	};

char *b_ground[3]={"home.bmp","14.bmp","r10.bmp"};//背景


char *music[8]={"./madplay d1.mp3 &",
				"./madplay d2.mp3 &",
				"./madplay d3.mp3 &",
				"./madplay d4.mp3 &",
				"./madplay d5.mp3 &",
				"./madplay d6.mp3 &",
				"./madplay d7.mp3 &",
				"./madplay d8.mp3 &",
		};

char *pia[2]={"12.bmp","15.bmp"};//钢琴黑白

char *video_name[] = {"1.avi", "2.avi","3.avi"};//视频文件



int screen_open (){//屏幕
	fd=open("/dev/fb0",O_RDWR);
	if(fd<0){
		perror("open screen failed");
		return -1;
	}
}

void screen_close(){
	close(fd);
}

void lcd_mmap(){//映射
	lcd = mmap(NULL,800*480*4,PROT_READ |PROT_WRITE,MAP_SHARED,fd,0);
}

void mmap_free(){
	munmap(lcd,800*480*4);
}

int touch (){
	//打开触摸屏
	touch_fd = open("/dev/input/event0",O_RDONLY);
	if(touch_fd < 0)
	{
		perror("open touch failed!");
		return -1;
	}
}


void touch_close(){
	close(touch_fd);
}

int pic_open(char *arg[],int p){
	bmpfd =open(arg[p],O_RDONLY);
		if(bmpfd<0){
			perror("open bmp failed");
			return -1;}
	
}

int show_bmp(int w){
	
		unsigned char head[54]={0};
		read(bmpfd,head,54);
	
		int kuan = *((int*)&head[18]);
		int gao = *((int*)&head[22]);
	
		
	
		int buf[gao][kuan];//32位
		char tmp_buf[gao*kuan*3];
	
		read(bmpfd,tmp_buf,sizeof(tmp_buf));
		
		unsigned char *p = tmp_buf;
	
	
		for(y=0;y<gao;y++)
			for(x=0;x<kuan;x++){
				b=*p++;
				g=*p++;
				r=*p++;
				a=0;
				color= a << 24 | r << 16 | g << 8 | b;
				buf[y][x] = color;
				
				*(lcd+(gao-1-y)*800+x+w*100)=buf[y][x];
				
			}

}

int if_slip(int x1,int x2,int p){
	if ( x1!=0 && x2!=0){

			
		if (x1>x2){  //右滑
			if (p<7)
				return ++p;
			else if (p==7)
				return 0;
			printf("右");

		}
		else if (x1<x2){	//左滑
			if (p>0)
				return --p;
			else if (p==0)
				return 7;
			printf("左");
	}}
	return p;
}


void bmp_close(){
	
	close(bmpfd);
}

void flash_white(){
	for(i=0;i<8;i++){
		pic_open(pia,0);
		show_bmp(i);
		bmp_close();
	}
}

void *get_xy(){//实时判断手指坐标
	touch();
	struct input_event  ts_val;//创建结构体模型
	while(1)
	{
		read(touch_fd,&ts_val,sizeof(ts_val));//监测是否有触摸数据产生
		
		//坐标轴数据
		if(ts_val.type == EV_ABS)
		{
			//X轴
			if(ts_val.code ==  ABS_X)
			{
				x_now = ts_val.value;
				printf("x_now=%d\n",x_now);
			}
			
			//Y轴
			if(ts_val.code ==  ABS_Y)
			{
				y_now = ts_val.value;
				printf("y_now=%d\n",y_now);
			}
			
			
		}
		
		if(ts_val.type == EV_KEY && ts_val.code == BTN_TOUCH )
			{				
				key = ts_val.value;
				printf("key = %d\n",key);
				if(ts_val.value == 1 ){
					x1=x_now;
					printf("x1=%d,",x1);
				}
			}
				
		if(ts_val.type == EV_KEY && ts_val.code == BTN_TOUCH )
		{//松手检测
				key = ts_val.value;
			
				if (ts_val.value == 0){
					//if ((x1-x_now)< -50 || (x1-x_now)>50){
					if (flag==1){
						pi=if_slip(x1,x_now,pi);
						//printf("pi=%d",pi);
					}
					if (flag==2){
						//system("killall -9 madplay");
						//printf("zanting\n");
						flash_white();
					}	
				}				
			}	
	}
		touch_close();
}


void show_home(){
	pic_open(b_ground,0);
	show_bmp(0);
	bmp_close;
}

int home_to(){
	while(1){
		if (key==0){
			if(x_now>0 && x_now<199 && y_now>140 && y_now<340 && flag ==0)
			{flag=1;album();}
			if(x_now>299 && x_now<499 && y_now>140 && y_now<340 && flag ==0)
			{flag=2;piano();}
		    if(x_now>599 && x_now<799 && y_now>140 && y_now<340 && flag ==0)
			{flag=3;video();}
		}
	}
	
	
}
void back(){
	//system("killall -9 madplay");
	printf("back0\n");
	flag=0;
	show_home();
	home_to();
}

void back1(){
	//system("killall -9 madplay");
	printf("back1\n");
	flag=0;
	show_home();
	
}

 void if_back(){
		if(x_now>=650&&x_now<800&&y_now>330&&y_now<=480&&flag!=0){
			back();	
		}	 
 }


int madplay(char *arg[],int p){
	system("killall -9 madplay");
	system(arg[p]);
	
}

void flash_blue(int w){
	pic_open(pia,1);
	show_bmp(w);
	bmp_close();
}

void show_piano(){//钢琴背景
	pic_open(b_ground,1);
	show_bmp(0);
	bmp_close();
	flash_white();
}

int where(int w){
	if(x_now>=0&&x_now<100){w=0;}
		else if(x_now>=100&&x_now<200){w=1;}
		else if(x_now>=200&&x_now<300){w=2;}
		else if(x_now>=300&&x_now<400){w=3;}
		else if(x_now>=400&&x_now<500){w=4;}
		else if(x_now>=500&&x_now<600){w=5;}
		else if(x_now>=600&&x_now<700){w=6;}
		else if(x_now>=700&&x_now<800){w=7;}
	return w;
}


void stop1(){//暂停循环
	while(key==1){
		if(key==0){
			break;
		}			
	}
}


void show_video(){
	pic_open(b_ground,2);
    show_bmp(0);
	bmp_close();//
}

int fifo_open(){//管道初始化
	//创建管道
	if(access("/tmp/fifo",F_OK))
	{
		//创建管道文件
		int ret = mkfifo("/tmp/fifo",0777);
		if(ret == -1)
		{
			printf("mkfifo error!\n");
		}
	}
	
	//以可读写方式打开管道
	fifo_fd = open("/tmp/fifo", O_RDWR);
	if(fifo_fd == -1)
	{
		perror("open fifo fail");
		return -1;
	}
	
	
	return fifo_fd;
	
}

void pause1(){//暂停/播放
		if(x_now>280 && x_now<380 && y_now>=380){
			write(fifo_fd, "pause\n", strlen("pause\n")); 	//暂停/播放
			sleep(1);	//延时1秒播放和暂停
			}
		}

int  next(int i){//下一首
	//下一个视频
			if(x_now>=560 && x_now<=660 &&  y_now>=380){
				if(i<2)
					return ++i;
				else if(i==2)
					return 0;
				sprintf(cmd1, "mplayer -slave -quiet -input  file=/tmp/fifo -geometry 40:40 -zoom -x 600 -y 360 %s &", video_name[i]);
				system("killall -9 mplayer &");	//结束已经打开的视频
				system(cmd1);	//播放下一首
				stop1();
			}
}

int last(int i){//上一首
		if(x_now>=0 && x_now<100 && y_now>=380){
				
				if(i>0)
					return --i;
				else if(i==0)
					return 2;
				sprintf(cmd1, "mplayer -slave -quiet -input  file=/tmp/fifo -geometry 40:40 -zoom -x 600 -y 360 %s &", video_name[i]);
				system("killall -9 mplayer &");	//结束已经打开的视频
				system(cmd1);	//播放上一首
				stop1();
			}
}


void fast(){//快进
	if(x_now>=420 && x_now<520 && y_now>=380){
				write(fifo_fd,"seek +10\n",strlen("seek +10\n"));//快进
				stop1();
			}
}

void slow(){
	if(x_now>=140 && x_now<240&& y_now>=380){
				write(fifo_fd,"seek -10\n",strlen("seek -10\n"));//快退
				stop1();
			}
}

void voice_up(){
	if(x_now>=670 && x_now<770 && y_now>=50 && y_now<=150){
				write(fifo_fd,"volume +10\n",strlen("volume +10\n"));//加音量
				stop1();
			}
}

void voice_down(){
	
			if(x_now>=670 && x_now<770 && y_now>=210 && y_now<=310){
				write(fifo_fd,"volume -10\n",strlen("volume -10\n"));//减音量
				stop1();
			}
	
}

void quit(){
	if(x_now>=700 && x_now<800 && y_now>=380){
				//write(fifo_fd,"quit\n",strlen("quit\n"));//退出
				system("killall -9 mplayer &");
				back();
				
				
			}
	
}

int album()
{   
	//printf("album\n");
	flag =1;
	flag2=0;
	//system("./madplay 1.mp3 &");
	while(1){
		
	//printf("p=%d\n",pi);
	pic_open(pic,pi);
	show_bmp(0);
	//printf("shuatu\n");
	if (key==1||flag2==1){
		printf("back\n");
		back1();break;}
	bmp_close();
	}
	
	return 0;
}	

int piano(){	
	int w=9;
	//sleep(1);
	show_piano();
	flag=2;
	while(1){
		if (key==1){
			if_back();
			if(y_now<=300){
				w = where(w);
				//蓝
				flash_blue(w);
				madplay(music,w);
				while(key==1){
					if(key==0){
						break;
						}
					}
				}
			}	
		}
	return 0;
}



int video(){
	show_video();
	
	fifo_fd=fifo_open();//管道初始化
	
	//播放初始音乐
	system("killall -9 mplayer &");	
	system("mplayer -slave -quiet -input  file=/tmp/fifo -geometry 40:40 -zoom -x 600 -y 360 1.avi &");
	
	int j=0;
	int x=0, y=0;
	flag2=0;
	
	while(1)
	{
		if (key==1||flag2==1){
		printf("videoback\n");
		system("killall -9 mplayer &");
		back1();
		break;
		}
		
		if(key==1){
			pause1();
			j=next(j);//下一首
			j=last(j);
			fast();
			slow();
			voice_up();
			voice_down();
			quit();
		}		
	}
	return 0;

}



//int flag2;
//封装初始化udp通信的函数
int udp_init()
{
	int ret; 
	//定义一个ipv4地址结构体变量
	struct sockaddr_in baddr;
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

/**


这个位置放获取触摸屏坐标函数
例如函数名为get_xy

显示图片等操作函数也可以放在这里

**/



// 录音并发送音频文件给电脑
void * send_wav(void *arg)
{
	char name[32]={0};
	char sbuf[1024];
	int open_flag=1;
	int fd;
	int ret;
	int addrsize=sizeof(struct sockaddr_in);
	//要发送的对方的IP和端口号
	struct sockaddr_in saddr;	
	bzero(&saddr,addrsize);	
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(50000);//端口号，同一台电脑上不同的程序需要使用不同的端口号
	saddr.sin_addr.s_addr=inet_addr( "192.168.0.200" );//电脑UBUNTU的IP
	
	while(1)
	{
		//点击屏幕任意位置进入录音
		if(x_now>0 && x_now<800 && y_now>0 && y_now<480 ){
		//调用system函数进行录音
		system("arecord  -d3  -c1  -r16000  -twav  -fS16_LE  1.wav");
		usleep(1000);//停顿一毫秒再发送给电脑Ubuntu
		//录音完毕就打开音频文件
		fd = open("1.wav",O_RDWR);
		if(fd<0)
		{
			perror("open fail");
			return -1;
		}
		while(1)
		{
			bzero(sbuf,1024);
			//读取音频文件数据
			ret = read(fd,sbuf,1024);
			if(ret<0)//读失败的话就报错退出
			{
				perror("read fail");
				return -1;
			}
			else if(ret == 0)//读完发送一个字符串表示结束，可以自己设置
			{		
				close(fd);
				break;
			}
			else//发送从文件中读取到的数据,读取到多少个字节就发送多少个字节
			{
				sendto(udpsock,sbuf,ret,0,(struct sockaddr *)&saddr,addrsize);
			}
			usleep(1000);	
		}	
		
	 }
	
	}
}




void * reciv(){
	char cmd[64]={0};//保存电脑Ubuntu回传的识别结果
	struct sockaddr_in raddr;//保存接收到网络数据的发送方的IP和端口号
	int addrsize=sizeof(struct sockaddr_in);
	bzero(&raddr,addrsize);

	while(1)
	{
		//printf("接受1\n");
		//接收电脑Ubuntu回传的识别结果
		recvfrom(udpsock,cmd,64,0,(struct sockaddr *)&raddr,&addrsize);
		//根据回传结果做出对应的操作
		if(strcmp(cmd,"xc") == 0)
		{
			
			flag=1;
			
			//printf("pic-flag2=%d",flag);
			//album();
			
		}
		if(strcmp(cmd,"gq") == 0)
		{
			flag=2;
		
			//piano();
			
		}
		if(strcmp(cmd,"sp") == 0)
		{
			
			flag=3;
			
			//video();
			
		}
		if(strcmp(cmd,"fh") == 0)
		{
			flag=0;
			flag2=1;
			printf("fh\n");
			//back();
			//printf("back-flag2=%d",flag2);
			
		}
		if(strcmp(cmd,"kj") == 0)
		{
			
			printf("kj\n");
			write(fifo_fd,"seek +10\n",strlen("seek +30\n"));
		}
		if(strcmp(cmd,"kt") == 0)
		{
			sprintf(cmd1, "mplayer -slave -quiet -input  file=/tmp/fifo -geometry 40:40 -zoom -x 600 -y 360 %s &", video_name[2]);
				system("killall -9 mplayer &");	//结束已经打开的视频
				system(cmd1);	//播放下一首
		}
		if(strcmp(cmd,"zt") == 0)
		{
			write(fifo_fd, "pause\n", strlen("pause\n")); 	//暂停/播放
			sleep(1);	//延时1秒播放和暂停
		}
		if(strcmp(cmd,"bf") == 0)
		{
			write(fifo_fd, "pause\n", strlen("pause\n")); 	//暂停/播放
			sleep(1);	//延时1秒播放和暂停
		}
		/**
		修改并添加你设定好的字段 并做出对应的操作
		**/
		
	
	}
}



int main(int argc,char **argv)
{
	 char cmd[64]={0};//保存电脑Ubuntu回传的识别结果
	
	struct sockaddr_in raddr;//保存接收到网络数据的发送方的IP和端口号
	int addrsize=sizeof(struct sockaddr_in);
	bzero(&raddr,addrsize); 
	pthread_t thread;
	/****
	添加打开触摸屏显示屏等设备代码
	
	******/
	screen_open();//打开屏幕
	lcd_mmap();//映射
	touch ();//打开触摸屏
	
	
	pthread_t pid;
	pthread_create(&pid,NULL,get_xy,NULL);
	
	//printf ("asd");
	
	
	
	
	
	//初始化网络
	udp_init();
	//开启线程，来点击屏幕录音
	pthread_create(&thread,NULL,send_wav,NULL);
	
	pthread_t rec;
	pthread_create(&rec,NULL,reciv,NULL);
	
	/**
	显示一个操作界面
	**/
	show_home();//主界面
	
	while(1){
		//printf("panduan\n");
		if(flag==1){printf("album1\n");album();}
		if(flag==2){printf("piano1\n");piano();}
		if(flag==3){printf("video1\n");video();}
		if(flag==0){printf("BACK\n");back1();}
		/*if(flag==1){album();}
		else 
		//
		else */
	
	};
   /*  while(1)
	{
		//接收电脑Ubuntu回传的识别结果
		recvfrom(udpsock,cmd,64,0,(struct sockaddr *)&raddr,&addrsize);
		//根据回传结果做出对应的操作
		if(strcmp(cmd,"xc") == 0)
		{
			
		
			
			printf("pic-flag2");
			album();
			
		}
		if(strcmp(cmd,"gq") == 0)
		{
			
		
			piano();
			
		}
		if(strcmp(cmd,"sp") == 0)
		{
			
			
			video();
			
		}
		if(strcmp(cmd,"fh") == 0)
		{
			
		
			
			printf("fanghui");
			back();
			//printf("back-flag2=%d",flag2);
			
		}
		/**
		修改并添加你设定好的字段 并做出对应的操作
		**/
		
	
	 //}  */
	screen_close();
	touch_close();
	bmp_close();
	mmap_free();
	
}