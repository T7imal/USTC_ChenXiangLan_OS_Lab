/*
 * 识别格式化字符串的核心代码写在本文件中
 * 可以从网上移植代码
 */ 
 
#include <stdarg.h>  

int vsprintf(char *buf, const char *fmt, va_list argptr){
	//只实现%d的格式化字符串的处理
	//填写正确的内容    
    char * str;

    for (str=buf ; *fmt ; ++fmt) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }
        
        ++fmt;
        switch (*fmt) {
            case 'd':
                int num=va_arg(argptr, int);
                if(num<0){
                    num=-num;
                    *str++='-';
                }
                char* numstr;
                int i=0;
                while(num>0){
                    int temp=num%10;
                    numstr[i++]=(char)temp+48;
                    num/=10;
                }
                while(i>0){
                    *str++=numstr[--i];
                }
                break;
            default:
                if (*fmt != '%')
                    *str++ = '%';
                if (*fmt)
                    *str++ = *fmt;
                else
                    --fmt;
                break;
        }
    }
    *str = '\0';
    
    return str-buf;
}
