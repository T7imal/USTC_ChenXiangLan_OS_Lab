#define WHITE 0x7
extern void taskEnd(void);
extern int createTask(void (*taskBody)(void));
extern int myPrintf(int color, const char* format, ...);

char* message1 = "********************************\n";

void myTask0(void) {
	myPrintf(WHITE, message1);
	myPrintf(WHITE, "*     Task0: HELLO WORLD!       *\n");
	myPrintf(WHITE, message1);

	taskEnd();   //the task is end
}

void myTask1(void) {
	myPrintf(WHITE, message1);
	myPrintf(WHITE, "*     Task1: HELLO WORLD!       *\n");
	myPrintf(WHITE, message1);

	taskEnd();   //the task is end
}

void myTask2(void) {
	myPrintf(WHITE, message1);
	myPrintf(WHITE, "*     Task2: HELLO WORLD!       *\n");
	myPrintf(WHITE, message1);

	taskEnd();  //the task is end
}
