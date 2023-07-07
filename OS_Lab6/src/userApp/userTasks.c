#include "../myOS/include/wallClock.h"

#define WHITE 0x7
#define RED 0x4
#define COLOR0 0x2
#define COLOR1 0x5
#define COLOR2 0x3
extern void taskEnd(void);
extern int createTask(void (*taskBody)(void));
extern int myPrintf(int color, const char* format, ...);

char* message1 = "********************************\n";

void myTask0(void) {
	myPrintf(WHITE, message1);
	myPrintf(WHITE, "*     Task0: TASK BEGIN!       *\n");
	int times = 0, temp = 0;
	while (times < 10) {
		if (temp != tick_number) {
			if (tick_number % 10 == 0) {
				myPrintf(COLOR0, "%d ", tick_number);
				times++;
			}
		}
		temp = tick_number;
	}
	myPrintf(WHITE, "\n");
	myPrintf(RED, "*     Task0: TASK END!       *\n");
	myPrintf(RED, message1);

	taskEnd();   //the task is end
}

void myTask1(void) {
	myPrintf(WHITE, message1);
	myPrintf(WHITE, "*     Task1: TASK BEGIN!       *\n");
	int times = 0, temp = 0;
	while (times < 10) {
		if (temp != tick_number) {
			if (tick_number % 10 == 0) {
				myPrintf(COLOR1, "%d ", tick_number);
				times++;
			}
		}
		temp = tick_number;
	}
	myPrintf(WHITE, "\n");
	myPrintf(RED, "*     Task1: TASK END!       *\n");
	myPrintf(RED, message1);

	taskEnd();   //the task is end
}

void myTask2(void) {
	myPrintf(WHITE, message1);
	myPrintf(WHITE, "*     Task2: TASK BEGIN!       *\n");
	int times = 0, temp = 0;
	while (times < 10) {
		if (temp != tick_number) {
			if (tick_number % 10 == 0) {
				myPrintf(COLOR2, "%d ", tick_number);
				times++;
			}
		}
		temp = tick_number;
	}
	myPrintf(WHITE, "\n");
	myPrintf(RED, "*     Task2: TASK END!       *\n");
	myPrintf(RED, message1);

	taskEnd();  //the task is end
}
