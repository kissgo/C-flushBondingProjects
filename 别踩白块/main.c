#include <stdio.h>
#include <pthread.h>
#include "a.h"

int main (){
	screen_open();//����Ļ
	lcd_mmap();//ӳ��
	btn();//�򿪰������ 
	show_start();//������
	
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
	
	
	
