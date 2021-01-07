#include  <reg52.h>	
#include  <math.h>    //Keil library  
#include  <stdio.h>   //Keil library	
#include  <INTRINS.H>
#include  <wenku.h>
#define   uchar unsigned char
#define   uint unsigned int	
sbit      sclk=P3^2; //8-SCK	串行时钟
sbit      sid=P3^1; //9-SDA	 串行数据
sbit      rs=P3^0; //10-RS	 寄存选择信号
sbit      reset=P1^0; //11-RST	复位  H：数据储存位 0：指令储存
sbit      cs1=P1^1; //12-CS		  低电平片选
sbit	  SCL=P1^2;      //IIC时钟引脚定义
sbit 	  SDA=P1^3;      //IIC数据引脚定义
sbit      key=P3^2;
sbit      en1=P1^4;
sbit      P36=P3^6;
sbit      s1=P3^5;
sbit      s2=P3^4;
sbit      s3=P3^3;
sbit      s4=P3^2;
#define	SlaveAddress   0xA6	  //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改
                              //ALT  ADDRESS引脚接地时地址为0xA6，接电源时地址为0x3A
typedef unsigned char  BYTE;
typedef unsigned short WORD;

BYTE BUF[8];                         //接收数据缓存区      	

float z,x,jiao;
int a1,a2,a3,a4;
int  dis_data;                       //变量
int  t=0,tmp=10,gu;
uint set_value=0;
uint feng=0;


void delay(unsigned int k);
void Init_ADXL345(void);             //初始化ADXL345

void WriteDataLCM(uchar dataW);
void WriteCommandLCM(uchar CMD,uchar Attribc);
void DisplayOneChar(uchar X,uchar Y,uchar DData);
void conversion(uint temp_data);

void  Single_Write_ADXL345(uchar REG_Address,uchar REG_data);   //单个写入数据
uchar Single_Read_ADXL345(uchar REG_Address);                   //单个读取内部寄存器数据
void  Multiple_Read_ADXL345();                                  //连续的读取内部寄存器数据
//------------------------------------
void Delay5us();
void Delay5ms();
void ADXL345_Start();
void ADXL345_Stop();
void ADXL345_SendACK(bit ack);
bit  ADXL345_RecvACK();
void ADXL345_SendByte(BYTE dat);
BYTE ADXL345_RecvByte();
void ADXL345_ReadPage();
void ADXL345_WritePage();
/***************lcd12864一系列指令！！！*************/
void transfer_command(int data1);
void transfer_data(int data1);
void delay_us(int i);
void initial_lcd();
void lcd_address(uchar page,uchar column);
void display_string_5x7(uint page,uint column,uchar *text);
void clear_screen();

/***************************************/
    void clear_screen()
	{
	unsigned char i,j;
	for(i=0;i<9;i++)
	 {
	lcd_address(1+i,1);for(j=0;j<132;j++)
	{
	transfer_data(0x00);
	}
	 }
	}

   void lcd_address(uchar page,uchar column)
   {
       column=column-1; //我们平常所说的第1 列，在LCD 驱动IC 里是第0 列。所以在这里减去1.
       page=page-1;
       transfer_command(0xb0+page);//设置页地址为page，0xb0是默认页地址为第0页，再加上输入的实际页码-1，就是实际输入页码的页地址 
       transfer_command(((column>>4)&0x0f)+0x10); //设置列地址的高4 位
       transfer_command(column&0x0f); //设置列地址的低4 位
   }


  void delay_us(int i)
  {
	int j,k;
	for(j=0;j<i;j++)
	for(k=0;k<1;k++);
  }
 void initial_lcd()
  {
    reset=0; /*低电平复位*/
    delay(100);
    reset=1; /*复位完毕*/
    delay(100);
    transfer_command(0xe2); /*软复位*/
    delay(5);
    transfer_command(0x2c); /*升压步聚1*/
    delay(5);
    transfer_command(0x2e); /*升压步聚2*/
    delay(5);
    transfer_command(0x2f); /*升压步聚3*/
    delay(5);
    transfer_command(0x24); /*粗调对比度，可设置范围0x20～0x27*/
    transfer_command(0x81); /*微调对比度*/
    transfer_command(0x2a); /*0x1a,微调对比度的值，可设置范围0x00～0x3f*/
    transfer_command(0xa2); /*1/9 偏压比（bias）*/
    transfer_command(0xc8); /*行扫描顺序：从上到下*/
    transfer_command(0xa0); /*列扫描顺序：从左到右*/
    transfer_command(0x40); /*起始行：第一行开始*/
    transfer_command(0xaf); /*开显示*/
  }




 void transfer_command(int data1)
 {
	char i;
	cs1=0;	//cs=0是指令储存，cs是寄存选择信号
	rs=0;	//打开片选
	for(i=0;i<8;i++)
	{
	sclk=0;
	delay_us(2);
	if(data1&0x80) sid=1;
	else sid=0;
	sclk=1;
	delay_us(2);
	data1=data1<<=1;
	}
	cs1=1;
 }



   /*写数据到lcd*/

void transfer_data(int data1)
{
   char i;  
   cs1=0; //cs=0是数据储存器
   rs=1;
    for(i=0;i<8;i++)
    {
      sclk=0;
      delay_us(1);
      if(data1&0x80) sid=1;
      else sid=0;
      sclk=1;
      delay_us(1);
      data1=data1<<=1;
    }
   cs1=1;
}

void display_string_5x7(uint page,uint column,uchar *text)
{
	uint i=0,j,k;
	while(text[i]>0x00)
	{
	if((text[i]>=0x20)&&(text[i]<0x7e))
	{
	j=text[i]-0x20;
	lcd_address(page,column);
	for(k=0;k<5;k++)
	{
	transfer_data(ascii_table_5x7[j][k]);/*显示5x7的ASCII字到LCD上，y为页地址，x 为列地址，最后为数据*/
	}
	i++;
	column+=6;
	}
	else
	i++;
	}
}
   void shuzi(uint page,uint column,uint i)
   {
      switch(i)
	  {
	     case 0:display_string_5x7(page,column,"0");break;
		 case 1:display_string_5x7(page,column,"1");break;
		 case 2:display_string_5x7(page,column,"2");break;
		 case 3:display_string_5x7(page,column,"3");break;
		 case 4:display_string_5x7(page,column,"4");break;
		 case 5:display_string_5x7(page,column,"5");break;
		 case 6:display_string_5x7(page,column,"6");break;
		 case 7:display_string_5x7(page,column,"7");break;
		 case 8:display_string_5x7(page,column,"8");break;
		 case 9:display_string_5x7(page,column,"9");break;
	  }
   }
/*******************************/
void delay(unsigned int k)	
{						
    unsigned int i,j;				
	for(i=0;i<k;i++)
	{			
	for(j=0;j<121;j++)			
	{;}}						
}
/*******************************/
				
/*******************************/
				

/**************************************
延时5微秒(STC90C52RC---12MHz---12T)
不同的工作环境,需要调整此函数，注意时钟过快时需要修改
当改用1T的MCU时,请调整此延时函数
**************************************/
void Delay5us()
{
    _nop_();_nop_();_nop_();_nop_();
    _nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();
}

/**************************************
延时5毫秒(STC90C52RC@12M)
不同的工作环境,需要调整此函数
当改用1T的MCU时,请调整此延时函数
**************************************/
void Delay5ms()
{
    WORD n = 560;

    while (n--);
}

/**************************************
起始信号
**************************************/
void ADXL345_Start()
{
    SDA = 1;                    //拉高数据线
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    SDA = 0;                    //产生下降沿
    Delay5us();                 //延时
    SCL = 0;                    //拉低时钟线
}

/**************************************
停止信号
**************************************/
void ADXL345_Stop()
{
    SDA = 0;                    //拉低数据线
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    SDA = 1;                    //产生上升沿
    Delay5us();                 //延时
}

/**************************************
发送应答信号
入口参数:ack (0:ACK 1:NAK)
**************************************/
void ADXL345_SendACK(bit ack)
{
    SDA = ack;                  //写应答信号
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    SCL = 0;                    //拉低时钟线
    Delay5us();                 //延时
}

/**************************************
接收应答信号
**************************************/
bit ADXL345_RecvACK()
{
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    CY = SDA;                   //读应答信号
    SCL = 0;                    //拉低时钟线
    Delay5us();                 //延时

    return CY;
}

/**************************************
向IIC总线发送一个字节数据
**************************************/
void ADXL345_SendByte(BYTE dat)
{
    BYTE i;

    for (i=0; i<8; i++)         //8位计数器
    {
        dat <<= 1;              //移出数据的最高位
        SDA = CY;               //送数据口
        SCL = 1;                //拉高时钟线
        Delay5us();             //延时
        SCL = 0;                //拉低时钟线
        Delay5us();             //延时
    }
    ADXL345_RecvACK();
}

/**************************************
从IIC总线接收一个字节数据
**************************************/
BYTE ADXL345_RecvByte()
{
    BYTE i;
    BYTE dat = 0;

    SDA = 1;                    //使能内部上拉,准备读取数据,
    for (i=0; i<8; i++)         //8位计数器
    {
        dat <<= 1;
        SCL = 1;                //拉高时钟线
        Delay5us();             //延时
        dat |= SDA;             //读数据               
        SCL = 0;                //拉低时钟线
        Delay5us();             //延时
    }
    return dat;
}

//******单字节写入*******************************************

void Single_Write_ADXL345(uchar REG_Address,uchar REG_data)
{
    ADXL345_Start();                  //起始信号
    ADXL345_SendByte(SlaveAddress);   //发送设备地址+写信号
    ADXL345_SendByte(REG_Address);    //内部寄存器地址，请参考中文pdf22页 
    ADXL345_SendByte(REG_data);       //内部寄存器数据，请参考中文pdf22页 
    ADXL345_Stop();                   //发送停止信号
}

//********单字节读取*****************************************
uchar Single_Read_ADXL345(uchar REG_Address)
{  uchar REG_data;
    ADXL345_Start();                          //起始信号
    ADXL345_SendByte(SlaveAddress);           //发送设备地址+写信号
    ADXL345_SendByte(REG_Address);            //发送存储单元地址，从0开始	
    ADXL345_Start();                          //起始信号
    ADXL345_SendByte(SlaveAddress+1);         //发送设备地址+读信号
    REG_data=ADXL345_RecvByte();              //读出寄存器数据
	ADXL345_SendACK(1);   
	ADXL345_Stop();                           //停止信号
    return REG_data; 
}
//*********************************************************
//
//连续读出ADXL345内部加速度数据，地址范围0x32~0x37
//
//*********************************************************
void Multiple_read_ADXL345(void)
{   uchar i;
    ADXL345_Start();                          //起始信号
    ADXL345_SendByte(SlaveAddress);           //发送设备地址+写信号
    ADXL345_SendByte(0x32);                   //发送存储单元地址，从0x32开始	
    ADXL345_Start();                          //起始信号
    ADXL345_SendByte(SlaveAddress+1);         //发送设备地址+读信号
	 for (i=0; i<6; i++)                      //连续读取6个地址数据，存储中BUF
    {
        BUF[i] = ADXL345_RecvByte();          //BUF[0]存储0x32地址中的数据
        if (i == 5)
        {
           ADXL345_SendACK(1);                //最后一个数据需要回NOACK
        }
        else
        {
          ADXL345_SendACK(0);                //回应ACK
       }
   }
    ADXL345_Stop();                          //停止信号
    Delay5ms();
}


//*****************************************************************

//初始化ADXL345，根据需要请参考pdf进行修改************************
void Init_ADXL345()
{
   Single_Write_ADXL345(0x31,0x0B);   //测量范围,正负16g，13位模式
   Single_Write_ADXL345(0x2C,0x08);   //速率设定为12.5 参考pdf13页
   Single_Write_ADXL345(0x2D,0x08);   //选择电源模式   参考pdf24页
   Single_Write_ADXL345(0x2E,0x80);   //使能 DATA_READY 中断
   Single_Write_ADXL345(0x1E,0x00);   //X 偏移量 根据测试传感器的状态写入pdf29页
   Single_Write_ADXL345(0x1F,0x00);   //Y 偏移量 根据测试传感器的状态写入pdf29页
   Single_Write_ADXL345(0x20,0x05);   //Z 偏移量 根据测试传感器的状态写入pdf29页
}

void qiujiao()
{ 
    Multiple_Read_ADXL345();       	//连续读出数据，存储在BUF中
    x=(abs)((BUF[1]<<8)+BUF[0]);
	z=(abs)((BUF[5]<<8)+BUF[4]);
	jiao=(atan(z/x)*180)/3.14159;
} 

   
void timer0() interrupt 1 /* T0中断服务程序 0.1ms一次中断*/ 
{
   if(t<tmp)//高电平时间
   {
     en1=1;
   } 
   if(t>=tmp&&t<100)//低电平时间
   {
	  en1=0;
   }
   if(t>=100)//周期控制
   {
      t=0;
   }
      	  t++;//中断一次加一次1，也就是0.1ms加一次

}
void chushi()
{
	TMOD=0x12; /* 设定T0的工作模式为2 */ 
	TH0=0x9B; /* 装入定时器的初值155 0.1ms*/ 
	TL0=0x9B; 
	EA=1; /* 开中断 */ 
	ET0=1; /* 定时器0允许中断 */ 
	TR0=1; /* 启动定时器0 */ 
}
void delayms(int x) //延时 x ms
{
 int i,j;
for(i=x;i>0;i--)
for(j=110;j>0;j--);
}
void display_jiao()
{
    qiujiao();			
	a1=	((int)jiao)%100/10;
	a2=	((int)jiao)%100%10;
	a3= (int) ((jiao-(int)jiao)/0.1) ;			
	shuzi(1,8*5,a1);
	shuzi(1,8*6,a2);
	display_string_5x7(1,8*7,".");
	shuzi(1,8*8,a3);	
   
}
//*********************************************************
//******主程序********
//*********************************************************
void main()
{ 
	 uchar devid;
	 delay(500);
    /*设定角度值*/
     initial_lcd();
	 clear_screen();
	 display_string_5x7(2,1,"s3 for wind control");
	  display_string_5x7(3,1,"s2 for 45 degrees");
	 display_string_5x7(4,1,"s4 for angle control");
 while(1)
 {
    if(s3==0)
    {
            Init_ADXL345();                 	//初始化ADXL345
		    devid=Single_Read_ADXL345(0X00);	//读出的数据为0XE5,表示
            initial_lcd();
			clear_screen();
			 chushi();
			 display_string_5x7(2,8*1,"wind:");
			 display_string_5x7(1,8*1,"now:");		
			while(1)
			{ 
			   display_jiao();
			   shuzi(2,8*6,tmp/10);
			   shuzi(2,8*7,tmp%10);
			   if(s1==0&&tmp<99)
			   {
			      tmp++;
			   }
			   if(s2==0&&tmp>0)
			   {
				  tmp--;
			   }   
			}   
     }
   else if(s2==0)
   {
             Init_ADXL345();                 	//初始化ADXL345
		     devid=Single_Read_ADXL345(0X00);	//读出的数据为0XE5,表示
             initial_lcd();
			 clear_screen();
			 chushi();
			 display_string_5x7(1,8*1,"now:");
			 while(1)
			 {
			     display_jiao();
				 if((int)jiao<45)
				 {
				    tmp++;
				 }
				 if((int)jiao>45)
				 {
				   tmp--;
				 }
				 if((int)jiao==45)
				 {
				    if((int)jiao>40&&(int)jiao<50)
					{
					   while(1)
					   {
						   P36=0;
						   delay(1);
						   P36=1;
						   delay(1);
						   qiujiao();
						   if((int)jiao>50||(int)jiao<40)
						   {
						     break;
						   }
						   else continue;
					   }
					}
				 }
			 }	
   }
   else if(s4==0)		                 			
	{		
	        
	       initial_lcd();
			clear_screen();
			while(1)
			{
			    display_string_5x7(2,8*1,"set:");
				shuzi(2,8*5,set_value/10);
				shuzi(2,8*6,set_value%10);
				if(s1==0&&set_value>=0&&set_value<60)
				{  
				   delay(60);
				   set_value++;
				}
				if(s2==0&&set_value>0&&set_value<=60)
				{  delay(60);
				   set_value--;
				}
				if(s3==0)
				{break;}
			}
			/**********/
			  chushi();//中断允许，定时器初始化			                        
			  Init_ADXL345();                 	//初始化ADXL345
			  devid=Single_Read_ADXL345(0X00);	//读出的数据为0XE5,表示正确
			  display_string_5x7(1,8*1,"now:");
			  display_string_5x7(2,8*1,"set:");
			while(1)                         	
			{ 	
				shuzi(2,8*5,set_value/10);
				shuzi(2,8*6,set_value%10);
				devid=Single_Read_ADXL345(0X00);	//读出的数据为0XE5,表示正确
				if(((int)jiao)<set_value)
				{
				   tmp++;
				}
				
				if(((int)jiao)>set_value)
				{	   
				   tmp--;
				}
			
				if((int)jiao==set_value)
				{
				   //delayms(400);//3000
				   if((int)jiao>=set_value-3&&(int)jiao<=set_value+3) //允许误差范围，防止条件太苛刻导致长时间调整不好
				   {
				     //gu=tmp;
				     while(1)
					 {
					   //tmp=gu;			   
					   P36=0;
					   delay(1);
					   P36=1;
					   delay(1);
					   qiujiao();
					   if(((int)jiao<set_value-3)||((int)jiao>set_value+3))
					   {
					      break;
					   }
					   else continue;		 			  
					 } 
				   }  
				}	   	
				if(devid!=0XE5)
				{	
				  display_string_5x7(1,1,"error!!!");			
				}
				else
				{			
				  display_jiao();					    
				}		
			}
		}

	}
} 
