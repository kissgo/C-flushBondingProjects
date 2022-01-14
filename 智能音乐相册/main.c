#include <stdio.h>
#include <pthread.h>
#include "a.h"

int main (){
	screen_open();//打开屏幕
	lcd_mmap();//映射
	touch ();//打开触摸屏
	show_home();//主界面
	pthread_t pid;
	pthread_create(&pid,NULL,get_xy,NULL);
	printf ("asd");
	home_to();
	screen_close();
	touch_close();
	bmp_close();
	mmap_free();
	return 0;
	}
	
	
	
