#ifndef  _a_H_
#define  _a_H_
int screen_open();//����Ļ
void screen_close();
void lcd_mmap();//ӳ��
void mmap_free();

int touch ();
void touch_close();
void *get_xy();

int btn();//�򿪰������ 
void btn_close();
int pic_open(char *arg[],int p);//��ĳ��ͼƬ
int show_bmp(int local);//д����򿪵�ͼƬ
void bmp_close();
void *get_press();//ʵʱ�жϰ���״̬
void show_start();//������
void show_end();

void flash(int a);//ˢ��һ�и���
void first();//��ʼ������
void show_all();//չʾ���и���
void drop();//
int is_right();//�ж��Ƿ���ȷ
int game();


#endif
