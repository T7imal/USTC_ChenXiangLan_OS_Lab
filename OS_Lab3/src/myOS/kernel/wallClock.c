void setWallClock(int HH,int MM,int SS){
	//你需要填写它
	char time[8];
	char* vga_p=(char*) 0xB8F90;
	time[0]=(char)(HH/10)+48;
	time[1]=(char)(HH%10)+48;
	time[2]=':';
	time[3]=(char)(MM/10)+48;
	time[4]=(char)(MM%10)+48;
	time[5]=':';
	time[6]=(char)(SS/10)+48;
	time[7]=(char)(SS%10)+48;
	for(int i=0;i<8;++i){
	    *vga_p++=time[i];
	    *vga_p++=0x20;
	}
}

void getWallClock(int *HH,int *MM,int *SS){
	//你需要填写它
	char time[10];
	char* vga_p=(char*) 0xB8F90;
	*HH=(*vga_p-48)*10+(*(vga_p+2)-48);
	vga_p+=6;
	*MM=(*vga_p-48)*10+(*(vga_p+2)-48);
	vga_p+=6;
	*SS=(*vga_p-48)*10+(*(vga_p+2)-48);
}
