#include <stdio.h>
#include <oled.h>

/******
限速位置01电机cmd
*******/
//u8 cmd01[12] = {0x01,0xFB,0x01,0x68,0xD8,0x00,0x00,0xB6,0xD0,0x00,0x01,0x6B};
//							地址 0xFB  方向    rpm   					raw   			mode 多机 结束符
//						 	 [0]  [1]  [2]    [3][4]    	[5][6][7][8]	  [9]  [10] [11]

/******
限速位置02电机cmd
*******/
//u8 cmd02[12] = {0x02,0xFB,0x00,0x68,0xD8,0x00,0x00,0xB6,0xD0,0x00,0x01,0x6B};
//							地址 0xFB  方向    rpm   					raw   			mode 多机 结束符
//						 	 [0]  [1]  [2]    [3][4]    	[5][6][7][8]	  [9]  [10] [11]

/******
梯形控制01电机cmd
*******/
u8 cmd01[16] = {0x01,0xFD,0x01,0x0a,0x22,0x0a,0x22,0x27,0x4c,0x00,0x00,0xB6,0xD0,0x00,0x01,0x6B};
//						 地址  0xFD 方向 加速度    减速度  		rpm				raw   			       mode 多机 结束符
//						 [0]   [1]  [2]	 [3][4]		 [5][6] 		[7][8]		[9][10][11][12]		 [13] [14]  [15]

/******
梯形控制02电机cmd
*******/
u8 cmd02[16] = {0x02,0xFD,0x00,0x0a,0x22,0x0a,0x22,0x27,0x4c,0x00,0x00,0xB6,0xD0,0x00,0x01,0x6B};
//						  地址 0xFD 方向 加速度    减速度  	 rpm						raw   			 mode 多机 结束符
//						  [0]  [1]  [2]	 [3][4]		 [5][6] 	 [7][8]		[9][10][11][12]		 [13] [14]  [15]

/******
同步运动开始指令
*******/
u8 cmd[4] = {0x00,0xFF,0x66,0x6B};

u8 Rxdate[4];
int i = 0;
int p = 1;
u8 t=' ';
bit busy;
bit send_flag;
bit dog = 1;
u32 worktime = 0;

void UartInit();

void sendchar(u8 c);
void send_string(u8 *p,u8 time);

void UartInit(void)		//38400bps@11.0592MHz
{
	SCON = 0x50;		//8Byte
	AUXR |= 0x01;		//UART1 use INT2
	AUXR |= 0x04;		//1T mode
	T2L = 0xB8;
	T2H = 0xFF;		//time set
	AUXR |= 0x10;		//start
	
	ES = 1;		//UART interrupt open
	EA = 1;		//All interrupt open
}

void sendchar(u8 c)
{
	while(busy);
	busy = 1;
	SBUF = c;
}

void send_string(u8 *p,u8 time)
{
	while(time != 0) 
	{
		sendchar(*p++);
		time --;
	}
}

void main()
{
	delay_ms(1000);	

	OLED_Init();//初始化OLED  
	OLED_ColorTurn(0);//0正常显示，1 反色显示
  OLED_DisplayTurn(1);//0正常显示 1 屏幕翻转显示
	
	send_flag = 1;//soft switch flag
	UartInit();//初始化UART串口
	OLED_Clear();
	OLED_Clear();
	OLED_ShowNum(0,0,worktime,5,16);	    //display worktime
	while(1)
	{
				if(send_flag == 1 && dog == 1) 			 //normal work mode
		{
			//send_string(cmd01,12);    			 //send cmd to motor01
			send_string(cmd01,16);   					 //send Trz cmd to motor01
			delay_ms(50);
			//send_string(cmd02,12);    		   //send cmd to motor02
			send_string(cmd02,16);   					 //send Trz cmd to motor02
			delay_ms(50);
			send_string(cmd,4);    						 //Synchronous motion
			
			
			cmd01[2] = cmd01[2] ^ 0x01;				 //Dir change01
			cmd02[2] = cmd02[2] ^ 0x01;				 //Dir change02
			delay_ms(2732);									   //waiting for motor
		}
		if(worktime >= 31622)  								 //mission complete
		{
		  send_flag = 0;
			ES = 1;														 //UART interrupt close
		}
	}
}

void UART() interrupt 4
{
	if(TI)                                 //send message
	{
		TI = 0;
		busy = 0;
	}
	if(RI)                                 //receive message
	{
		RI = 0;
		Rxdate[i] = SBUF;                    //read message[02 FD 02 6B]
		i++;
		if(i == 4)                           //message length
		{
			if(Rxdate[0] == 0x02 && Rxdate[2] == 0x02)
			{
				worktime ++;
				OLED_Clear();
				OLED_ShowNum(0,0,worktime,5,16); //display worktime
			}
			i = 0;
			Rxdate[0] = 0x00;
			Rxdate[2] = 0x00;
		}
	}
}
