/*	
	* Function List:
	*	1. void I2C_Configuration(void) -- 配置CPU的硬件I2C
	* 2. void I2C_WriteByte(uint8_t addr,uint8_t data) -- 向寄存器地址写一个byte的数据
	* 3. void WriteCmd(unsigned char I2C_Command) -- 写命令
	* 4. void WriteDat(unsigned char I2C_Data) -- 写数据
	* 5. void OLED_Init(void) -- OLED屏初始化
	* 6. void OLED_SetPos(unsigned char x, unsigned char y) -- 设置起始点坐标
	* 7. void OLED_Fill(unsigned char fill_Data) -- 全屏填充
	* 8. void OLED_CLS(void) -- 清屏
	* 9. void OLED_ON(void) -- 唤醒
	* 10. void OLED_OFF(void) -- 睡眠
	* 11. void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) -- 显示字符串(字体大小有6*8和8*16两种)
	* 12. void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N) -- 显示中文(中文需要先取模，然后放到codetab.h中)
	* 13. void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]) -- BMP图片
	*
  *
  ******************************************************************************
  */ 




#include "OLED_I2C.h"
#include "codetab.h"
#include "./systick/bsp_SysTick.h"
#include "menu.h"

extern uint8_t migong[16][32];

extern uint8_t OLED_temp[64][128];

uint8_t OLED_jieshi [8][128]={0};


 /**
  * @brief  I2C_Configuration，初始化硬件IIC引脚
  * @param  无
  * @retval 无
  */
void I2C_Configuration(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure; 

	/*I2C1外设时钟使能 */
	OLED_I2C_CLK_INIT(OLED_I2C_CLK,ENABLE);
	
	/*I2C1外设GPIO时钟使能 */
	RCC_APB2PeriphClockCmd(OLED_I2C_SCL_GPIO_CLK | OLED_I2C_SDA_GPIO_CLK,ENABLE);

	 /* I2C_SCL、I2C_SDA*/
  GPIO_InitStructure.GPIO_Pin = OLED_I2C_SCL_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;	       // 开漏输出
  GPIO_Init(OLED_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = OLED_I2C_SDA_PIN;
  GPIO_Init(OLED_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);	
	
	
	/* I2C 配置 */
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;	
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;	/* 高电平数据稳定，低电平数据变化 SCL 时钟线的占空比 */
  I2C_InitStructure.I2C_OwnAddress1 =OLED_ADDRESS;    //主机的I2C地址
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable ;	
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	/* I2C的寻址模式 */
  I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;	                            /* 通信速率 */
  
  I2C_Init(OLED_I2C, &I2C_InitStructure);	                                      /* I2C1 初始化 */
	I2C_Cmd(OLED_I2C, ENABLE);  	                                                /* 使能 I2C1 */
	
}


 /**
  * @brief  I2C_WriteByte，向OLED寄存器地址写一个byte的数据
  * @param  addr：寄存器地址
	*					data：要写入的数据
  * @retval 无
  */
void I2C_WriteByte(uint8_t addr,uint8_t data)
{
  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(I2C1, ENABLE);//开启I2C1
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,主模式*/

	I2C_Send7bitAddress(I2C1, OLED_ADDRESS, I2C_Direction_Transmitter);//器件地址 -- 默认0x78
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C1, addr);//寄存器地址
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_SendData(I2C1, data);//发送数据
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTOP(I2C1, ENABLE);//关闭I2C1总线
}


 /**
  * @brief  WriteCmd，向OLED写入命令
  * @param  I2C_Command：命令代码
  * @retval 无
  */
void WriteCmd(unsigned char I2C_Command)//写命令
{
	I2C_WriteByte(0x00, I2C_Command);
}


 /**
  * @brief  WriteDat，向OLED写入数据
  * @param  I2C_Data：数据
  * @retval 无
  */
void WriteDat(unsigned char I2C_Data)//写数据
{
	I2C_WriteByte(0x40, I2C_Data);
}


 /**
  * @brief  OLED_Init，初始化OLED
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	Delay_s(1);		// 1s,这里的延时很重要,上电后延时，没有错误的冗余设计
	
	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode	
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
}


 /**
  * @brief  OLED_SetPos，设置光标
  * @param  x,光标x位置
	*					y，光标y位置
  * @retval 无
  */
void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

 /**
  * @brief  OLED_Fill，填充整个屏幕
  * @param  fill_Data:要填充的数据
	* @retval 无
  */
void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);		//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				WriteDat(fill_Data);
			}
	}
}

 /**
  * @brief  OLED_CLS，清屏
  * @param  无
	* @retval 无
  */
void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}


 /**
  * @brief  OLED_ON，将OLED从休眠中唤醒
  * @param  无
	* @retval 无
  */
void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}


 /**
  * @brief  OLED_OFF，让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
  * @param  无
	* @retval 无
  */
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}


 /**
  * @brief  OLED_ShowStr，显示codetab.h中的ASCII字符,有6*8和8*16可选择
  * @param  x,y : 起始点坐标(x:0~127, y:0~7);
	*					ch[] :- 要显示的字符串; 
	*					TextSize : 字符大小(1:6*8 ; 2:8*16)
	* @retval 无
  */
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

 /**
  * @brief  OLED_ShowCN，显示codetab.h中的汉字,16*16点阵
  * @param  x,y: 起始点坐标(x:0~127, y:0~7); 
	*					N:汉字在codetab.h中的索引
	* @retval 无
  */
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=32*N;
	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
}



 /**
  * @brief  OLED_DrawBMP，显示BMP位图
  * @param  x0,y0 :起始点坐标(x0:0~127, y0:0~7);
	*					x1,y1 : 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
	* @retval 无
  */
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}

//设置空白
void OLED_Showspace(unsigned char x, unsigned char y, unsigned char TextSize){
	unsigned char c = 0,i = 0;
	switch(TextSize)
	{
		case 1:
		{
			
				c = space;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				
				
			
		}break;
		case 2:
		{
			
				c = space;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
			
				
			
		}break;
	}
}

//设置箭头
void OLED_Showflag(unsigned char x, unsigned char y, unsigned char TextSize){
	unsigned char c = 0,i = 0;
	switch(TextSize)
	{
		case 1:
		{
			
				c = 30;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				
				
			
		}break;
		case 2:
		{
			
				c = 30;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
			
				
			
		}break;
	}
}

//在指定坐标放置一个4x4的方阵
//x=1~32
//y=1~16
void OLED_Set4squre(uint8_t x,uint8_t y){
  uint8_t i=0;
  x=x-1,y=y-1;
  if((y&1)==0) {  //y-1奇数
    if(x==0) OLED_SetPos(0,y/2);
    else OLED_SetPos(x*4-1,y/2) ;
    for(i=0;i<4;i++){
       WriteDat(0x0f);
    }
  }
  else {
    if(x==0) OLED_SetPos(0,y/2);
    else OLED_SetPos(x*4-1,y/2);
    for(i=0;i<4;i++){
       WriteDat(0xf0);
    }
  }
  

}

// 在屏幕上指定坐标亮起一个像素点
void OLED_DrawPixel(uint8_t x, uint8_t y)
{   
    x=x-1,y=y-1;
    // 计算像素所在的页和列
    uint8_t page = y / 8;
    uint8_t column = x;

    // 计算像素在页内的偏移量
    uint8_t offset = y % 8;

    // 设置页地址
    OLED_SetPos(column,page);

    

    // 亮起像素点
    WriteDat(1 << offset);
}


// 在屏幕上指定坐标显示一个16x16的中文字符
void OLED_Draw16x16Chinese(uint8_t x, uint8_t y, unsigned char N)
{
    // 计算像素所在的页和列
    uint8_t page = y / 8,i=0,core=0,addr=32*N;
    uint8_t column = x;
    unsigned char up[16]={""},mid[16]={""},down[16]={""};
    
    

    // 计算像素在页内的偏移量
    uint8_t offset = y % 8;
   //上页
   for(i=0;i<16;i++){
      core=(0xFF>>offset);
      up[i]=((F16x16[addr+i]|core)<<offset);
}
  //中页上
  for(i=0;i<16;i++){
      core=(0xFF<<(8-offset));
      mid[i]=((F16x16[addr+i]|core)>>(8-offset));
}
  //中页下
  for(i=0;i<16;i++){
      core=(0xFF<<(offset));
      mid[i]=(((F16x16[addr+16+i]|core)|(0xFF>>(8-offset)))|mid[i]);}
  //下页
  for(i=0;i<16;i++){
      core=(0xFF>>(8-offset));
      mid[i]=((F16x16[addr+16+i]|core)<<(8-offset));
}

  OLED_SetPos(column,page);
  for(i=0;i<16;i++){
      WriteDat(up[i]);
}
  OLED_SetPos(column,page+1);  
  for(i=0;i<16;i++){
      WriteDat(mid[i]);
}
  OLED_SetPos(column,page+2);  
  for(i=0;i<16;i++){
      WriteDat(down[i]);
}
}    
void OLED_Set4square_mg(uint8_t x,uint8_t y){
  uint8_t i=0;
  uint8_t x_t=x-1,y_t=y-1;
  if((y_t&1)==0) {  //y奇数
    if(x_t==0) OLED_SetPos(0,y_t/2);
    else OLED_SetPos(x_t*4+1,y_t/2) ;
   
      if(migong[y-1+1][x-1]==1)  for(i=0;i<4;i++){WriteDat(0xff);}
      else for(i=0;i<4;i++){WriteDat(0x0f);}
    
  }
  else {
    if(x_t==0) OLED_SetPos(0,y_t/2);
    else OLED_SetPos(x_t*4+1,y_t/2);
   {
       if(migong[y-1-1][x-1]==1) { for(i=0;i<4;i++) {WriteDat(0xff); }}
       else for(i=0;i<4;i++){WriteDat(0xf0);}
    }
  }

}
   
void OLED_gettemp() {
  uint8_t a=0,i=0,j=0,e=0;
  for(i=0;i<8;i++){
    for(j=0;j<128;j++){
      for(e=0;e<8;e++){
        //if(e==0) a=OLED_temp[i*8+(8-e-1)][j]|a<<1;
        //a=(0xff|OLED_temp[i*8+(8-e-1)][j])&((a<<1)|1);
        a=a|(OLED_temp[i*8+(8-e-1)][j]<<(8-e-1));
}
      OLED_jieshi[i][j]=a;
      a=0;
}
    
}
  
  for(i=0;i<8;i++){
    OLED_SetPos(0,i);
    for(j=0;j<128;j++) WriteDat(OLED_jieshi[i][j]);
    

}

}

void OLED_temp_CLS(){
  uint8_t i=0,j=0;
  for(i=0;i<64;i++){
    for(j=0;j<128;j++) OLED_temp[i][j]=0;

}
}

void ch_temp(int16_t x,uint8_t y,uint8_t num){
  unsigned int  adder=32*num;
  uint8_t i=0,j=0,a=1,e=0;
  if(y==0) y=32;
  if(x>=128) return;
  if(x<0) {
    adder=adder-x;
    for(i=0;i<2;i++){
    for(j=0;j<16+x;j++){
      for(e=0;e<8;e++){
        if((F16x16[adder]&(a<<e))!=0) OLED_temp[y+e+i*8][j]=1;
      }
      adder++;
    }
    adder=adder-x;
  }
}
  else if(x>(128-16)) {
    for(i=0;i<2;i++){
    for(j=0;j<16-(16-(128-x));j++){
      for(e=0;e<8;e++){
        if((F16x16[adder]&(a<<e))!=0) OLED_temp[y+e+i*8][x+j]=1;
      }
      adder++;
    }
    adder=adder+(16-(128-x));
  }

}

  
  else for(i=0;i<2;i++){
    for(j=0;j<16;j++){
      for(e=0;e<8;e++){
        if((F16x16[adder]&(a<<e))!=0) OLED_temp[y+e+i*8][x+j]=1;
      }
      adder++;
    }
  }
  
}

void num_temp(int16_t x,uint8_t y,uint8_t num){
  unsigned int  adder=num*16;
  uint8_t i=0,j=0,a=1,e=0;
  if(y==0) y=32;
  if(x>=128) return;
  if(x<0) {
    adder=adder-x;
    for(i=0;i<2;i++){
    for(j=0;j<8+x;j++){
      for(e=0;e<8;e++){
        if((F8X16[adder]&(a<<e))!=0) OLED_temp[y+e+i*8][j]=1;
      }
      adder++;
    }
    adder=adder-x;
  }
}
  else if(x>(128-8)) {
    for(i=0;i<2;i++){
    for(j=0;j<8-(8-(128-x));j++){
      for(e=0;e<8;e++){
        if((F8X16[adder]&(a<<e))!=0) OLED_temp[y+e+i*8][x+j]=1;
      }
      adder++;
    }
    adder=adder+(8-(128-x));
  }

}

  
  else for(i=0;i<2;i++){
    for(j=0;j<8;j++){
      for(e=0;e<8;e++){
        if((F8X16[adder]&(a<<e))!=0) OLED_temp[y+e+i*8][x+j]=1;
      }
      adder++;
    }
  }
  
}
   

