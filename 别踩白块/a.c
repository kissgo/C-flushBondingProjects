#include <fcntl.h>
#include <unistd.h>//close、read、write
#include <string.h>//strlen
#include <linux/input.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>



int fd;
int btn_fd;
int bmpfd ;
int *lcd ;
int x_now=0,x2=-1;//用来暂存按钮的值

int sx=0,sy=0;
int sx1=0,sy1=0;
int key=1;
int touch_fd;

int pi=0;//bmp指针
int flag=0; //进入界面标志

int map[4][4];

struct blank{//保存各个格子坐标 
	int x;
	int y;
	int a;
};





/* struct blank blanks[16]={{0,0},{0,255},{0，511},{0，767}
						,{149，0},{149,255},{149,511},{149,767}
						,{299,0},{299,255},{299,511},{299,767}
						,{449,0},{449,255},{449,511},{449,767}}; */
						
struct blank blanks[16]={{0,0},{0,255},{0,511},{0,767}
						,{149,0},{149,255},{149,511},{149,767}
						,{299,0},{299,255},{299,511},{299,767}
						,{449,0},{449,255},{449,511},{449,767}};

unsigned char r;
unsigned char g;
unsigned char b;
unsigned char a;
unsigned int color =0;
unsigned int x=0,y=0;

char *pic[3]={//白红键背景图 
		"white.bmp",
		"black.bmp",
		"red.bmp"	
	};

char *b_ground[2]={"start.bmp","end.bmp"};//背景





int screen_open (){//屏幕
	fd=open("/dev/fb0",O_RDWR);
	if(fd<0){
		perror("open screen failed");
		return -1;
	}
}

void screen_close(){//关闭屏幕 
	close(fd);
}

void lcd_mmap(){//映射
	lcd = mmap(NULL,1024*600*4,PROT_READ |PROT_WRITE,MAP_SHARED,fd,0);
}

void mmap_free(){//释放 
	munmap(lcd,1024*600*4);
}

int touch (){
	//打开触摸屏
	touch_fd = open("/dev/input/event2",O_RDONLY);
	if(touch_fd < 0)
	{
		perror("open touch failed!");
		return -1;
	}
}


void touch_close(){
	close(touch_fd);
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
				sx = ts_val.value;
				printf("sx=%d\n",sx);
			}
			
			//Y轴
			if(ts_val.code ==  ABS_Y)
			{
				sy = ts_val.value;
				printf("sy=%d\n",sy);
			}
			
			
		}
		
		if(ts_val.type == EV_KEY && ts_val.code == BTN_TOUCH )
			{
				
				key = ts_val.value;
				printf("key = %d\n",key);
				if(ts_val.value == 1 ){
					sx1=sx;
					printf("sx1=%d,",sx1);
				
				}
			}
				
		if(ts_val.type == EV_KEY && ts_val.code == BTN_TOUCH )
		{//松手检测
				key = ts_val.value;
			
				if (ts_val.value == 0){
					
						while(flag== 1){
							if(sx>0 && sx<255 && sy>0 && sy<149){
								x2=0;
								break;}
							if(sx>0 && sx<255 && sy>149 && sy<299){
								x2=1;
								break;}
							if(sx>0 && sx<255 && sy>299 && sy<449){
								x2=2;
								break;}
							if(sx>0 && sx<255 && sy>449 && sy<599){
								x2=3;
								break;}
							
	}	
				}
				
				
			}
		
	}
		touch_close();
}


int btn(){//打开按键检测 
	
	btn_fd = open("/dev/input/event0",O_RDONLY);
	if(btn_fd < 0)
	{
		perror("open btn failed!");
		return -1;
	}
}


void btn_close(){
	close(btn_fd);
}

int pic_open(char *arg[],int p){
	bmpfd =open(arg[p],O_RDONLY);
		if(bmpfd<0){
			perror("open bmp failed");
			return -1;}
	
}


int show_bmp(int local){//传入-1 就是左上角刷图 传入0-15代表格子坐标
		int w=0, h=0;
		if(local >= 0){
			w =blanks[local].y;
			//printf("w=%d\n",w);
			h = blanks[local].x;
			//printf("h=%d\n",h);
		}
		unsigned char head[54]={0};
		read(bmpfd,head,54);
	
		int kuan = *((int*)&head[18]);
		int gao = *((int*)&head[22]);
	
		
	
		int buf[gao][kuan];//32位
		char tmp_buf[gao*kuan*3];
	
		read(bmpfd,tmp_buf,sizeof(tmp_buf));
		
		unsigned char *p = tmp_buf;
	
	
		for(y=h;y<gao+h;y++)
			for(x=w;x<kuan+w;x++){
				b=*p++;
				g=*p++;
				r=*p++;
				a=0;
				color= a << 24 | r << 16 | g << 8 | b;
				buf[y-h][x-w] = color;
				
				*(lcd+(gao+h*2-1-y)*1024+x)=buf[y-h][x-w];
				
			}

}



void bmp_close(){
	
	close(bmpfd);
}


void *get_press(){//实时判断按键状态
	btn();
	struct input_event  ev_key;//创建结构体模型
	while(1)
	{
		read(btn_fd,&ev_key,sizeof(ev_key));//监测是否有按键数据产生
		
		if(ev_key.type == EV_KEY)
		{
			
			if(ev_key.value ==  1)
			{
				
				x_now = ev_key.code;
				printf("x_now=%d\n",x_now);
				
				//break;
				//continue;
			}
			if(ev_key.value ==  0)
			{
				x_now = 0;
				
				//break;
				//printf("x_now=%d\n",x_now);
					//continue;
			}
			
		}
	}
	
		btn_close();
}


void show_start(){
	pic_open(b_ground,0);
	show_bmp(-1);
	bmp_close;
}

void show_end(){
	pic_open(b_ground,1);
	show_bmp(-1);
	bmp_close;
	while(1){
		//x2=x_now;
		if(x_now==30)
		{	
			flag=0;
			game();
			break;
		}
	}
}

void flash(int a){//刷新一行格子
	int temp = rand()%4;
	int i;
	for(i=0;i<4;i++){
		map[i][a]=0;
	}
	map[temp][a]=1;
	
}

void first(){//初始化格子
	int j ;
	for(j=0;j<4;j++){
		flash(j);
		printf("j =%d\n",j);
	}
}

void show_all(){//展示所有格子
	int i ,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			
			//printf("map i j =%d\n",map[i][j]);
			//printf("i*4+j =%d\n",i*4+j);
			pic_open(pic,map[i][j]);
			show_bmp(i*4+j);
			
			bmp_close();
		}
	}
	
}
void drop()
{		
	int i;
	int j;
	for (j = 1; j < 4; j++)
	{
		for ( i = 0; i < 4; i++)
		{
			map[i][j-1] = map[i][j];
		}
	}

}


int touch_right(){//判断触摸是否按正确
	int i=0,j=0,k=-1,l;
	for(i=0;i<4;i++){
		if (map[i][0]==1)
		break;
	}

	//printf("j=%d\n",j);
	//x2=x_now;
	if(x2==i)
		return 1;
	else if(x2!=i)
	{	
	if(x2!=-1){
		for(l=0;l<5;l++){
		pic_open(pic,2);
		show_bmp(4*x2);
		bmp_close();
		sleep(0.2);
		pic_open(pic,1);
		show_bmp(4*x2);
		bmp_close();
		sleep(0.2);
		}
		return 0;
	}}
}


	
	
int is_right(){//判断是否按正确
	int i=0,j=0,k=-1,l;
	for(i=0;i<4;i++){
		if (map[i][0]==1)
		break;
	}
	switch(i){
		case 0:
			j=105;
			break;
		case 1:
			j=108;
			break;
		case 2:
			j=106;
			break;
		case 3:
			j=48;
			break;	
	}
	//printf("j=%d\n",j);
	//x2=x_now;
	if(x_now==j)
		return 1;
	else if(x_now!=j)
	{	
		switch(x_now){
		case 105:
			k=0;
			break;
		case 108:
			k=1;
			break;
		case 106:
			k=2;
			break;
		case 48:
			k=3;
			break;	
	}
	if(k!=-1){
		for(l=0;l<5;l++){
		pic_open(pic,2);
		show_bmp(4*k);
		bmp_close();
		sleep(0.2);
		pic_open(pic,1);
		show_bmp(4*k);
		bmp_close();
		sleep(0.2);
		}
		return 0;
	}}
}


int game(){
	show_start();
	while(flag==0){
	//x2=x_now;
	if(x_now==103){
		first();
		show_all();
		flag =1;

	}
	}
	while(flag== 1){
		//printf("x_now=%d\n",x_now);
		//printf("right=%d\n",is_right());
		//x2=x_now;
		if(x_now!=0&&is_right()==0){
			
			flag=2;
			break;
		}
		//printf("x_now1=%d\n",x_now);
		//while(x_now!=0&&x2==1&&is_right()==1){
		if(x_now!=0&&is_right()==1){
		x_now=0;
		drop();
		flash(3);
		show_all();
		}
		if(x2>=0){
			if(touch_right()){
				x2=-1;
				drop();
				flash(3);
				show_all();
			}
			else if (!touch_right()){
				x2=-1;
				flag=2;
				break;
			}
		}
		
	}
	
	show_end();
	
}







