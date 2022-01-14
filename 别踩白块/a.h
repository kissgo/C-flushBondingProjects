#ifndef  _a_H_
#define  _a_H_
int screen_open();//打开屏幕
void screen_close();
void lcd_mmap();//映射
void mmap_free();

int touch ();
void touch_close();
void *get_xy();

int btn();//打开按键检测 
void btn_close();
int pic_open(char *arg[],int p);//打开某张图片
int show_bmp(int local);//写上面打开的图片
void bmp_close();
void *get_press();//实时判断按键状态
void show_start();//主界面
void show_end();

void flash(int a);//刷新一行格子
void first();//初始化格子
void show_all();//展示所有格子
void drop();//
int is_right();//判断是否按正确
int game();


#endif
