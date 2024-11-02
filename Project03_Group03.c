#include <msp430.h> 
#include "uart.h"
#include "i2c.h"
#include "lcd.h"
#include <math.h>

#define I2C_ADDRESS_MPU9150 0x68

unsigned int gTick_0_5ms=0;

unsigned char buffer[10];

enum
{
    ROLL,
    PITCH
};


void main(void)
{
    int res=-1;
    WDTCTL = WDT_MDLY_0_5; //


    P1SEL0 &=~ (BIT0 + BIT1);
    P1SEL1 &=~ (BIT0 + BIT1);

    P1DIR |= BIT0; // output
    P1DIR &= ~BIT1; // input
    P1OUT |= BIT1;
    P1REN |= BIT1;
    PM5CTL0 &= ~ LOCKLPM5;

    SFRIE1 |= WDTIE;
        UART_Init(SPEED_38400_SMCLK);
        I2C_Init();
        LCD_Init();
        _EINT();

        char Display = ROLL;

        buffer[0]=0x80;
        res = I2C_WriteBlock(I2C_ADDRESS_MPU9150, buffer,1,0x6B);
        if (res!=I2C_SUCCESS)
        {
            LCD_displayShortHex("W1",res);
            return;
        }
        gTick_0_5ms = 100;
        LPM0;

        buffer[0]=0x00;
        res = I2C_WriteBlock(I2C_ADDRESS_MPU9150, buffer,1,0x6B);
        if (res!=I2C_SUCCESS)
        {
            LCD_displayShortHex("W2",res);
        }
        gTick_0_5ms = 100;
        LPM0;


    while(1)
    {
        res = I2C_ReadBlock(I2C_ADDRESS_MPU9150, buffer,6,0x3B);

        if (res!=I2C_SUCCESS)
        {
            LCD_displayShortHex("RD",res);
        }
        else
        {
            buffer[6]=0xA0;
            buffer[7]=0xB0;
            TX_ByteArray(buffer,8);
        }


        // Accel_X = buffer[0] + buffer[1]
        // Accel_Y = buffer[2] + buffer[3]
        // Accel_Z = buffer[4] + buffer[5]

        int Accel_X = (int)(((buffer[0] & 0xF) << 8) | buffer[1]);
        int Accel_Y = (int)(((buffer[2] & 0xF) << 8) | buffer[3]);
        int Accel_Z = (int)(((buffer[4] & 0xF) << 8) | buffer[5]);

        float RAD_TO_DEG = 57.29578;
        float roll = atan2(Accel_Y, Accel_Z) * RAD_TO_DEG;
        float pitch = atan2(Accel_X, Accel_Z) * RAD_TO_DEG;


        if ((P1IN & BIT1) == BIT1)
        {
            LCD_displayShort(roll);
        }

        else
        {
            LCD_displayShort(pitch);
        }
    }
}
