#include "RM.h"
#include "IIC.h"
#include <stdint.h>

void Start(void)
{
    SDA_High();
    SCL_High();
    
    Delay_ms(5);
    
    SDA_Low();
    SCL_Low();
}

void Stop(void)
{
    SDA_Low();
    SCL_High();

    Delay_ms(5);

    SDA_High();
}

void SendByte(uint8_t data)
{
    uint8_t i;

    for ( i = 0 ; i < 8 ; i++ )
    {
        if( data & 0x80 )
        {
            SDA_High();
        }
        else
        {
            SDA_Low();
        }

        Delay_ms(5);

        SCL_High();

        Delay_ms(5);

        SCL_Low();

        data <<= 1;
    }
}

uint8_t ReceiveAck(void)
{
    uint8_t ack;

    SDA_High();

    SCL_High();

    Delay_ms(5);

    ack = SDA_Read();

    SCL_Low();

    return ack;
}

void SendData(uint8_t data)
{
    Start();
    SendByte(data);
    ReceiveAck();
    Stop();
}

