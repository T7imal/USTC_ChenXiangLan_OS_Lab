/*
 * 本文件实现vga的相关功能，清屏和屏幕输出
 * clear_screen和append2screen必须按照如下借口实现
 * 可以增加其他函数供clear_screen和append2screen调用
 */
extern void outb (unsigned short int port_to, unsigned char value);
extern unsigned char inb(unsigned short int port_from);
//VGA字符界面规格：25行80列
//VGA显存初始地址为0xB8000

short cur_line=0;
short cur_column=0;//当前光标位置
char * vga_init_p=(char *)0xB8000;


void update_cursor(void){//通过当前行值cur_line与列值cur_column回写光标
	//填写正确的内容    
	short pos=cur_line*80+cur_column;
	if(pos>=25*80) return;
	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char)((pos&0xFF00)>>8));//输入高8bit
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char)(pos&0x00FF));//输入低8bit
}

short get_cursor_position(void){//获得当前光标，计算出cur_line和cur_column的值
	//填写正确的内容   
	short pos_low, pos_high, pos;
	outb(0x3D4, 0x0E);
	pos_high=inb(0x3D5);
	outb(0x3D4, 0x0F);
	pos_low=inb(0x3D5);
	pos=(pos_high<<8)+pos_low;
	cur_line=pos/80;
	cur_column=pos%80;
	return pos;
}


void clear_screen(void) {
	//填写正确的内容    
	unsigned char* vga_p=vga_init_p;
	for(int i=0;i<25*80;++i){
	    *vga_p++=' ';
	    *vga_p++=0x00;
	}
	cur_line=0;
	cur_column=0;
	update_cursor();
}

void scroll_screen(){
    unsigned char* vga_p=vga_init_p;
    for(;vga_p<0xB8F00;){
        *vga_p=*(vga_p+80*2);
        vga_p++;
        *vga_p=*(vga_p+80*2);
        vga_p++;
    }
    for(;vga_p<0xB8FA0;){
        *vga_p++=' ';
        *vga_p++=0x00;
    }
    cur_line=24;
    cur_column=0;
    update_cursor();
}

void put_char(unsigned char ch, int color){
    unsigned char* vga_p=vga_init_p;
    short pos=get_cursor_position();
    if(ch=='\n'){
        ++cur_line;
        cur_column=0;
        pos=cur_line*80+cur_column;
    }
    else if(pos<25*80){
        vga_p+=pos*2;
        *vga_p++=ch;
        *vga_p=(unsigned char)color;
        if(++cur_column==80){
            ++cur_line;
            cur_column=0;
        }
        pos=cur_line*80+cur_column;
    }
    if(pos>=25*80){
        scroll_screen();
    }
    update_cursor();
}

void append2screen(char *str,int color){ 
	//填写正确的内容    
	for(int i=0;str[i]!='\0';++i){
	    put_char(str[i], color);
	}
}




