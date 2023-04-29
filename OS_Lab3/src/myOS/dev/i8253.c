#include "io.h"

void init8253(void){
	//你需要填写它
	outb(0x43, 0x34);
	outb(0x40, 0x9c);
	outb(0x40, 0x2e);
}
//– 端口地址 0x40~0x43；
//– 14,3178 MHz crystal
//4,772,727 Hz system clock
//1,193,180 Hz to 8253
//– 设定时钟中断的频率为100HZ，分频参数是多少？
//– 初始化序列为：
//• 0x34 ==》端口0x43（参见控制字说明）
//• 分频参数==》端口0x40，分两次，先低8位，后高8位