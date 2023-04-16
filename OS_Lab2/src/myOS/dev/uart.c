/* 
 * 与 UART 相关的输出
 * 调用inb和outb函数，实现下面的uart的三个函数
 */
extern unsigned char inb(unsigned short int port_from);
extern void outb (unsigned short int port_to, unsigned char value);

#define uart_base 0x3F8

void uart_put_char(unsigned char c){
	//填写正确的内容
	outb(uart_base, c);
}

unsigned char uart_get_char(void){
	//填写正确的内容     
    return inb(uart_base);     
}

void uart_put_chars(char *str){ 
	//填写正确的内容    
	for(int i=0;str[i]!='\0';++i){
	    uart_put_char(str[i]);
	}
}
