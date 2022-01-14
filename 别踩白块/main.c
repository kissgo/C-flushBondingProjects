#include <stdio.h>
#include <pthread.h>
#include "a.h"

int main (){
	screen_open();//打开屏幕
	lcd_mmap();//映射
	btn();//打开按键检测 
	show_start();//主界面
	
	pthread_t pid;
	pthread_create(&pid,NULL,get_press,NULL);
	pthread_t pid1;
	pthread_create(&pid1,NULL,get_xy,NULL);
	//printf ("asd");
	
	game();
	
	screen_close();
	btn_close();
	bmp_close();
	mmap_free();
	return 0;
	
	
	}
	
	
	
