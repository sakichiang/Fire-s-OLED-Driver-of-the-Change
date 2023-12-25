/*	
	* Function List:
	*	1. void I2C_Configuration(void) -- ����CPU��Ӳ��I2C
	* 2. void I2C_WriteByte(uint8_t addr,uint8_t data) -- ��Ĵ�����ַдһ��byte������
	* 3. void WriteCmd(unsigned char I2C_Command) -- д����
	* 4. void WriteDat(unsigned char I2C_Data) -- д����
	* 5. void OLED_Init(void) -- OLED����ʼ��
	* 6. void OLED_SetPos(unsigned char x, unsigned char y) -- ������ʼ������
	* 7. void OLED_Fill(unsigned char fill_Data) -- ȫ�����
	* 8. void OLED_CLS(void) -- ����
	* 9. void OLED_ON(void) -- ����
	* 10. void OLED_OFF(void) -- ˯��
	* 11. void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) -- ��ʾ�ַ���(�����С��6*8��8*16����)
	* 12. void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N) -- ��ʾ����(������Ҫ��ȡģ��Ȼ��ŵ�codetab.h��)
	* 13. void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]) -- BMPͼƬ
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
  * @brief  I2C_Configuration����ʼ��Ӳ��IIC����
  * @param  ��
  * @retval ��
  */
void I2C_Configuration(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure; 

	/*I2C1����ʱ��ʹ�� */
	OLED_I2C_CLK_INIT(OLED_I2C_CLK,ENABLE);
	
	/*I2C1����GPIOʱ��ʹ�� */
	RCC_APB2PeriphClockCmd(OLED_I2C_SCL_GPIO_CLK | OLED_I2C_SDA_GPIO_CLK,ENABLE);

	 /* I2C_SCL��I2C_SDA*/
  GPIO_InitStructure.GPIO_Pin = OLED_I2C_SCL_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;	       // ��©���
  GPIO_Init(OLED_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = OLED_I2C_SDA_PIN;
  GPIO_Init(OLED_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);	
	
	
	/* I2C ���� */
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;	
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;	/* �ߵ�ƽ�����ȶ����͵�ƽ���ݱ仯 SCL ʱ���ߵ�ռ�ձ� */
  I2C_InitStructure.I2C_OwnAddress1 =OLED_ADDRESS;    //������I2C��ַ
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable ;	
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	/* I2C��Ѱַģʽ */
  I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;	                            /* ͨ������ */
  
  I2C_Init(OLED_I2C, &I2C_InitStructure);	                                      /* I2C1 ��ʼ�� */
	I2C_Cmd(OLED_I2C, ENABLE);  	                                                /* ʹ�� I2C1 */
	
}


 /**
  * @brief  I2C_WriteByte����OLED�Ĵ�����ַдһ��byte������
  * @param  addr���Ĵ�����ַ
	*					data��Ҫд�������
  * @retval ��
  */
void I2C_WriteByte(uint8_t addr,uint8_t data)
{
  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(I2C1, ENABLE);//����I2C1
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,��ģʽ*/

	I2C_Send7bitAddress(I2C1, OLED_ADDRESS, I2C_Direction_Transmitter);//������ַ -- Ĭ��0x78
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C1, addr);//�Ĵ�����ַ
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_SendData(I2C1, data);//��������
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTOP(I2C1, ENABLE);//�ر�I2C1����
}


 /**
  * @brief  WriteCmd����OLEDд������
  * @param  I2C_Command���������
  * @retval ��
  */
void WriteCmd(unsigned char I2C_Command)//д����
{
	I2C_WriteByte(0x00, I2C_Command);
}


 /**
  * @brief  WriteDat����OLEDд������
  * @param  I2C_Data������
  * @retval ��
  */
void WriteDat(unsigned char I2C_Data)//д����
{
	I2C_WriteByte(0x40, I2C_Data);
}


 /**
  * @brief  OLED_Init����ʼ��OLED
  * @param  ��
  * @retval ��
  */
void OLED_Init(void)
{
	Delay_s(1);		// 1s,�������ʱ����Ҫ,�ϵ����ʱ��û�д�����������
	
	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode	
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //���ȵ��� 0x00~0xff
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
  * @brief  OLED_SetPos�����ù��
  * @param  x,���xλ��
	*					y�����yλ��
  * @retval ��
  */
void OLED_SetPos(unsigned char x, unsigned char y) //������ʼ������
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

 /**
  * @brief  OLED_Fill�����������Ļ
  * @param  fill_Data:Ҫ��������
	* @retval ��
  */
void OLED_Fill(unsigned char fill_Data)//ȫ�����
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
  * @brief  OLED_CLS������
  * @param  ��
	* @retval ��
  */
void OLED_CLS(void)//����
{
	OLED_Fill(0x00);
}


 /**
  * @brief  OLED_ON����OLED�������л���
  * @param  ��
	* @retval ��
  */
void OLED_ON(void)
{
	WriteCmd(0X8D);  //���õ�ɱ�
	WriteCmd(0X14);  //������ɱ�
	WriteCmd(0XAF);  //OLED����
}


 /**
  * @brief  OLED_OFF����OLED���� -- ����ģʽ��,OLED���Ĳ���10uA
  * @param  ��
	* @retval ��
  */
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //���õ�ɱ�
	WriteCmd(0X10);  //�رյ�ɱ�
	WriteCmd(0XAE);  //OLED����
}


 /**
  * @brief  OLED_ShowStr����ʾcodetab.h�е�ASCII�ַ�,��6*8��8*16��ѡ��
  * @param  x,y : ��ʼ������(x:0~127, y:0~7);
	*					ch[] :- Ҫ��ʾ���ַ���; 
	*					TextSize : �ַ���С(1:6*8 ; 2:8*16)
	* @retval ��
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
  * @brief  OLED_ShowCN����ʾcodetab.h�еĺ���,16*16����
  * @param  x,y: ��ʼ������(x:0~127, y:0~7); 
	*					N:������codetab.h�е�����
	* @retval ��
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
  * @brief  OLED_DrawBMP����ʾBMPλͼ
  * @param  x0,y0 :��ʼ������(x0:0~127, y0:0~7);
	*					x1,y1 : ���Խ���(������)������(x1:1~128,y1:1~8)
	* @retval ��
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

//���ÿհ�
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

//���ü�ͷ
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

//��ָ���������һ��4x4�ķ���
//x=1~32
//y=1~16
void OLED_Set4squre(uint8_t x,uint8_t y){
  uint8_t i=0;
  x=x-1,y=y-1;
  if((y&1)==0) {  //y-1����
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

// ����Ļ��ָ����������һ�����ص�
void OLED_DrawPixel(uint8_t x, uint8_t y)
{   
    x=x-1,y=y-1;
    // �����������ڵ�ҳ����
    uint8_t page = y / 8;
    uint8_t column = x;

    // ����������ҳ�ڵ�ƫ����
    uint8_t offset = y % 8;

    // ����ҳ��ַ
    OLED_SetPos(column,page);

    

    // �������ص�
    WriteDat(1 << offset);
}


// ����Ļ��ָ��������ʾһ��16x16�������ַ�
void OLED_Draw16x16Chinese(uint8_t x, uint8_t y, unsigned char N)
{
    // �����������ڵ�ҳ����
    uint8_t page = y / 8,i=0,core=0,addr=32*N;
    uint8_t column = x;
    unsigned char up[16]={""},mid[16]={""},down[16]={""};
    
    

    // ����������ҳ�ڵ�ƫ����
    uint8_t offset = y % 8;
   //��ҳ
   for(i=0;i<16;i++){
      core=(0xFF>>offset);
      up[i]=((F16x16[addr+i]|core)<<offset);
}
  //��ҳ��
  for(i=0;i<16;i++){
      core=(0xFF<<(8-offset));
      mid[i]=((F16x16[addr+i]|core)>>(8-offset));
}
  //��ҳ��
  for(i=0;i<16;i++){
      core=(0xFF<<(offset));
      mid[i]=(((F16x16[addr+16+i]|core)|(0xFF>>(8-offset)))|mid[i]);}
  //��ҳ
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
  if((y_t&1)==0) {  //y����
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
   
