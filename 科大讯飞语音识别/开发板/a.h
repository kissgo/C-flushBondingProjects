#ifndef  _a_H_
#define  _a_H_
int screen_open();//打开屏幕
void screen_close();
void lcd_mmap();//映射
void mmap_free();
int touch ();//打开触摸屏
void touch_close();
int pic_open(char *pic[],int p);//打开某张图片
int show_bmp(int w);//写上面打开的图片
void if_slip(int x1,int x2,int p);//判断滑动
void bmp_close();
void *get_xy();//实时判断手指坐标
void show_home();//主界面
int home_to();
int album();//电子相册
int piano();
int madplay(char *arg[],int p);//音频播放
void back();
void if_back();
void flash_white();//刷白格
void flash_blue(int w);//刷蓝格
void show_piano();//钢琴背景
int where(int w);//琴键位置 
int video();//视频播放
void show_video();
int fifo_open();//管道初始化
void pause1();//暂停/播放
void stop1();//暂停循环
int next(int i);//下一首
int last(int i);//上一首
void fast();//快进
void slow();
void voice_up();
void voice_down();
void quit();
#endif