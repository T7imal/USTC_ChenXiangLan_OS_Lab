#include "wallClock.h"
int system_ticks;
int HH=0,MM=0,SS=0;

void tick(void){
	system_ticks++;
	//你需要填写它
	if(system_ticks==100){
	    system_ticks=0;
	    if(++SS==60){
	        SS=0;
	        if(++MM==60){
	            MM=0;
	            ++HH;
	        }
	    }
	}
	setWallClock(HH,MM,SS);
	return;
}

