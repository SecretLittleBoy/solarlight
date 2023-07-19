/*
 *
 * Description:128*64����OLEDģ������
 *
 * Others: none;
 *
 * Function List:
 *
 * 1. void delay(unsigned int z) -- ��ʱ����,����
 * 2. void IIC_Start() -- ����I2C����
 * 3. void IIC_Stop() -- �ر�I2C����
 * 4. void Write_IIC_Byte(unsigned char IIC_Byte) -- ͨ��I2C����дһ��byte������
 * 5. void OLED_WrDat(unsigned char dat) -- ��OLED��д����
 * 6. void OLED_WrCmd(unsigned char cmd) -- ��OLED��д����
 * 7. void OLED_Set_Pos(unsigned char x, unsigned char y) -- ������ʾ����
 * 8. void OLED_Fill(unsigned char bmp_dat) -- ȫ����ʾ(��ʾBMPͼƬʱ�Ż��õ��˹���)
 * 9. void OLED_CLS(void) -- ��λ/����
 * 10. void OLED_Init(void) -- OLED����ʼ�����򣬴˺���Ӧ�ڲ�����Ļ֮ǰ���ȵ���
 * 11. void OLED_P6x8Str(unsigned char x, y,unsigned char ch[]) -- 6x8������������ʾASCII�����С���У���̫����
 * 12. void OLED_P8x16Str(unsigned char x, y,unsigned char ch[]) -- 8x16������������ʾASCII�룬�ǳ�����
 * 13.void OLED_P16x16Ch(unsigned char x, y, N) -- 16x16������������ʾ���ֵ���С���У������ø������塢�Ӵ֡���б���»��ߵ�
 * 14.void Draw_BMP(unsigned char x0, y0,x1, y1,unsigned char BMP[]) -- ��128x64���ص�BMPλͼ��ȡ������������ֱ���Ȼ���Ƶ�codetab�У��˺������ü���
 *
 * History: none;
 *
 *************************************************************************************/

#include <stdlib.h>

#include "OLED12864.h"
#include "codetab.h"
#include "i2c.h"
#include "main.h"
#define Brightness 0xCF
#define X_WIDTH 128
#define Y_WIDTH 64
/*********************OLED���������õ���ʱ����************************************/
void delay(unsigned int z) {
    unsigned int x, y;
    for (x = z; x > 0; x--)
        for (y = 110; y > 0; y--)
            ;
}

/*********************OLEDд����************************************/
//After writing, the x position will be increased by 1 automatically
void OLED_WrDat(unsigned char IIC_Data) {
    HAL_I2C_Mem_Write(&hi2c2, 0x78, 0x40, I2C_MEMADD_SIZE_8BIT, &IIC_Data, 1, 100);
}
/*********************OLEDд����************************************/
void OLED_WrCmd(unsigned char IIC_Command) {
    HAL_I2C_Mem_Write(&hi2c2, 0x78, 0x00, I2C_MEMADD_SIZE_8BIT, &IIC_Command, 1, 100);
}

/*********************OLED ��������***********************************
(0,0)
 |-------------------> X  (0~127)
 |
 |
 |
\/
y (0~7)
*/
void OLED_Set_Pos(unsigned char x, unsigned char y) {
    OLED_WrCmd(0xb0 + y);
    OLED_WrCmd(((x & 0xf0) >> 4) | 0x10);
    OLED_WrCmd((x & 0x0f) | 0x01);
}
/*********************OLEDȫ��************************************/
void OLED_Fill(unsigned char bmp_dat) {
    unsigned char y, x;
    for (y = 0; y < 8; y++) {
        OLED_WrCmd(0xb0 + y);
        OLED_WrCmd(0x00);
        OLED_WrCmd(0x10);
        for (x = 0; x < X_WIDTH; x++)
            OLED_WrDat(bmp_dat);
    }
}
/*********************OLED����************************************/
void OLED_Clear(void) {
    unsigned char y, x;
    for (y = 0; y < 8; y++) {
        OLED_WrCmd(0xb0 + y);
        OLED_WrCmd(0x01);
        OLED_WrCmd(0x10);
        for (x = 0; x < X_WIDTH; x++)
            OLED_WrDat(0);
    }
}
/*********************OLED����Ļ************************************/
void OLED_ON(void) {
    OLED_WrCmd(0X8D);  // ���õ�ɱ�
    OLED_WrCmd(0X14);  // ������ɱ�
    OLED_WrCmd(0XAF);  // OLED����
}
/*********************OLED����Ļ************************************/
void OLED_OFF(void) {
    OLED_WrCmd(0X8D);  // ���õ�ɱ�
    OLED_WrCmd(0X10);  // �رյ�ɱ�
    OLED_WrCmd(0XAE);  // OLED����
}
/*********************OLED��ɫ��ʾ************************************/
void OLED_ColorTurn(unsigned char i) {
    if (i == 0) {
        OLED_WrCmd(0xA6);  // ������ʾ
    } else if (i == 1) {
        OLED_WrCmd(0xA7);  // ��ɫ��ʾ
    }
}
/*********************OLED��Ļ��ת180��************************************/
void OLED_DisplayTurn(unsigned char i) {
    if (i == 0) {
        OLED_WrCmd(0xC8);  // ������ʾ
        OLED_WrCmd(0xA1);
    } else if (i == 1) {
        OLED_WrCmd(0xC0);  // ��ת��ʾ
        OLED_WrCmd(0xA0);
    }
}
/*********************OLED��ʼ��************************************/
void OLED_Init(void) {
    delay(500);        // ��ʼ��֮ǰ����ʱ����Ҫ��
    OLED_WrCmd(0xae);  //--turn off oled panel
    OLED_WrCmd(0x00);  //---set low column address
    OLED_WrCmd(0x10);  //---set high column address
    OLED_WrCmd(0x40);  //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)

    OLED_WrCmd(0x81);        //--set contrast control register
    OLED_WrCmd(Brightness);  // Set SEG Output Current Brightness

    OLED_WrCmd(0xa1);  //--Set SEG/Column Mapping     0xa0���ҷ��� 0xa1����
    OLED_WrCmd(0xc8);  // Set COM/Row Scan Direction   0xc0���·��� 0xc8����
    OLED_WrCmd(0xa6);  //--set normal display
    OLED_WrCmd(0xa8);  //--set multiplex ratio(1 to 64)
    OLED_WrCmd(0x3f);  //--1/64 duty
    OLED_WrCmd(0xd3);  //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WrCmd(0x00);  //-not offset
    OLED_WrCmd(0xd5);  //--set display clock divide ratio/oscillator frequency
    OLED_WrCmd(0x80);  //--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WrCmd(0xd9);  //--set pre-charge period
    OLED_WrCmd(0xf1);  // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WrCmd(0xda);  //--set com pins hardware configuration
    OLED_WrCmd(0x12);
    OLED_WrCmd(0xdb);  //--set vcomh
    OLED_WrCmd(0x40);  // Set VCOM Deselect Level
    OLED_WrCmd(0x20);  //-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WrCmd(0x02);  //
    OLED_WrCmd(0x8d);  //--set Charge Pump enable/disable
    OLED_WrCmd(0x14);  //--set(0x10) disable
    OLED_WrCmd(0xa4);  // Disable Entire Display On (0xa4/0xa5)
    OLED_WrCmd(0xa6);  // Disable Inverse Display On (0xa6/a7)
    OLED_WrCmd(0xaf);  //--turn on oled panel
    OLED_Fill(0x00);   // ��ʼ����
    OLED_Set_Pos(0, 0);
}

/*****������������ʾ8*16��һλASCII�ַ�����ʾ�����꣨x,y����yΪҳ��Χ0��7****************/
void OLED_P8x16Char(unsigned char x, unsigned char y, unsigned char ch) {
    unsigned char i = 0;
    unsigned char c = ch - 32;
    OLED_Set_Pos(x, y);
    for (i = 0; i < 8; i++)
        OLED_WrDat(F8X16[16 * c + i]);
    OLED_Set_Pos(x, y + 1);
    for (i = 0; i < 8; i++)
        OLED_WrDat(F8X16[16 * c + i + 8]);
}

/****������������ʾ16*16�����ֻ�8*16 ASCII�ַ�����ʾ�����꣨x,y����
               x��Χ0~119��yΪҳ��Χ0��7**********************/
void OLED_ShowStr_HZ(unsigned char x, unsigned char y, unsigned char *str) {
    unsigned int k = 0;
    unsigned char j;
    while (*str)  // ����δ����
    {
        if (*str < 128)  // ASCII�ַ�
        {
            if (x > 120) {
                x = 0;
                y += 2;
            }  // 128-8=120��һ�е���󣬲���һ���ַ����ȣ��л�����һ�У��ӵ�һ�п�ʼ��ʾ
            OLED_P8x16Char(x, y, *str);
            x = x + 8;
            str++;
        } else  // ���ĺ���
        {
            if (x > 112) {
                x = 0;
                y += 2;
            }                           // 128-16=112��һ�е���󣬲���һ�����ֿ��ȣ��л�����һ�У��ӵ�һ�п�ʼ��ʾ
            for (k = 0; k < 3550; k++)  // �������к��ֿ⣬����Ա�
            {
                if ((GB_16[k].Index[0] == *(str)) && (GB_16[k].Index[1] == *(str + 1)))  // ����Աȳɹ�
                {
                    OLED_Set_Pos(x, y);  // ����x���꣬����ҳ��ַ����ʾ���ֵ��ϰ벿��
                    for (j = 0; j < 16; j++) {
                        OLED_WrDat(GB_16[k].Msk[j]);
                    }
                    OLED_Set_Pos(x, y + 1);  // ����x���꣬ҳ��1����ʾ�����°벿��
                    for (j = 0; j < 16; j++) {
                        OLED_WrDat(GB_16[k].Msk[j + 16]);
                    }
                    x = x + 16;
                    break;  // ���ҵ���Ӧ�����ֿ������˳�����������һ������
                }
            }
            str += 2;  // ÿ�����ֶ�Ӧ�����ַ���������ָ���2
        }
    }
}
/****������������ʾ��ʾBMPͼƬ128��64��ʼ������(x,y),x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7*****************
*****sizex��ͼƬ�Ŀ��ȷ�Χ1��128��sizeyΪͼƬ�ĸ߶ȷ�Χ1��64*****************
*/
void Draw_BMP(unsigned char x, unsigned char y, unsigned char sizex, unsigned char sizey, const char BMP[]) {
    unsigned int j = 0;
    unsigned char dx, dy;

    if (sizey % 8 == 0)  // ��ͼƬ�ĸ߶�ת����ҳ����ÿ8����һҳ
        sizey = sizey / 8;
    else
        sizey = sizey / 8 + 1;      // �����������ҳ�Ͷ��1.���Ƕ����ҲҪ��һҳ����
    for (dy = 0; dy < sizey; dy++)  // ��ҳȥд
    {
        OLED_Set_Pos(x, y + dy);  //
        for (dx = 0; dx < sizex; dx++) {
            OLED_WrDat(BMP[j++]);
        }
    }
}

/***************������������ʾ6*8һ���׼ASCII�ַ��� ��ʾ�����꣨x,y����yΪҳ��Χ0��7****************/
void OLED_P6x8Str(unsigned char x, unsigned char y, unsigned char ch[]) {
    unsigned char c = 0, i = 0, j = 0;
    while (ch[j] != '\0') {
        c = ch[j] - 32;
        if (x > 122) {
            x = 0;
            y++;
        }  // 128-6=122��һ�е���󣬲���һ���ַ����ȣ��л�����һ�У��ӵ�һ�п�ʼ��ʾ
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WrDat(F6x8[c][i]);
        x += 6;
        j++;
    }
}

///*******************������������ʾ8*16��һλ���֣���ʾ�����꣨x,y����yΪҳ��Χ0��7****************/
// void OLED_P8x16NUM(unsigned char x, unsigned char y,unsigned char NUM)
//{
//	unsigned char i=0;
//	OLED_Set_Pos(x,y);
//	for(i=0;i<8;i++)
//		OLED_WrDat(F8X16[16*16+NUM*16+i]);
//	OLED_Set_Pos(x,y+1);
//	for(i=0;i<8;i++)
//		OLED_WrDat(F8X16[16*16+NUM*16+i+8]);
// }

/****������������ʾ16*32��ASCII�ַ�������ʾ�����꣨x,y����yΪҳ��Χ0��7****************/
void OLED_P16X32Str(unsigned char x, unsigned char y, unsigned char *str) {
    unsigned char i = 0;
    unsigned char c;
    while (*str) {
        c = *str - 32;
        OLED_Set_Pos(x, y);
        for (i = 0; i < 16; i++)
            OLED_WrDat(F16X32[c * 64 + i]);
        OLED_Set_Pos(x, y + 1);
        for (i = 0; i < 16; i++)
            OLED_WrDat(F16X32[c * 64 + i + 16]);
        OLED_Set_Pos(x, y + 2);
        for (i = 0; i < 16; i++)
            OLED_WrDat(F16X32[c * 64 + i + 32]);
        OLED_Set_Pos(x, y + 3);
        for (i = 0; i < 16; i++)
            OLED_WrDat(F16X32[c * 64 + i + 48]);
        x += 16;
        str++;
    }
}

void my_P6x8Str(uint8_t x, uint8_t y, unsigned char ch1[], uint16_t num, unsigned char ch2[]) {  // num should  0-9999
    unsigned char c = 0, i = 0, j = 0;
    unsigned char *num2char = (unsigned char *)malloc(4 * sizeof(unsigned char));
    num2char[0] = num / 1000 + '0';
    num2char[1] = num / 100 % 10 + '0';
    num2char[2] = num / 10 % 10 + '0';
    num2char[3] = num % 10 + '0';
    while (ch1[j] != '\0') {
        c = ch1[j] - 32;
        if (x > 122) {
            x = 0;
            y++;
        }  // 128-6=122��һ�е���󣬲���һ���ַ����ȣ��л�����һ�У��ӵ�һ�п�ʼ��ʾ
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WrDat(F6x8[c][i]);
        x += 6;
        j++;
    }
    j = 0;
    if (num2char[0] == '0') {  // the '0' in the front of the number will not be displayed
        j++;
        if (num2char[1] == '0') {
            j++;
            if (num2char[2] == '0') {
                j++;
            }
        }
    }

    while (j <= 3) {
        c = num2char[j] - 32;
        if (x > 122) {
            x = 0;
            y++;
        }  // 128-6=122��һ�е���󣬲���һ���ַ����ȣ��л�����һ�У��ӵ�һ�п�ʼ��ʾ
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WrDat(F6x8[c][i]);
        x += 6;
        j++;
    }
    j = 0;
    while (ch2[j] != '\0') {
        c = ch2[j] - 32;
        if (x > 122) {
            x = 0;
            y++;
        }  // 128-6=122��һ�е���󣬲���һ���ַ����ȣ��л�����һ�У��ӵ�һ�п�ʼ��ʾ
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WrDat(F6x8[c][i]);
        x += 6;
        j++;
    }
    free(num2char);
}

void OLED_draw_battary_icon(uint8_t x, uint8_t y, uint8_t percent) {
    uint8_t i = 0;
    OLED_Set_Pos(x, y);
    OLED_WrDat(0xFF);
    for (; i < 12; i++) {
        if (i < percent) {
            OLED_WrDat(0xFF);
        } else {
            OLED_WrDat(0x81);
        }
    }
    if (percent >= 13) {
        OLED_WrDat(0xFF);
    } else {
        OLED_WrDat(0xE7);
    }
    if (percent >= 14) {
        OLED_WrDat(0x3C);
    } else {
        OLED_WrDat(0x24);
    }
    OLED_WrDat(0x3C);
}