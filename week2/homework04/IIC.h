#ifndef __IIC_H
#define __IIC_H

#include "RM.h"

void Start(void);
void Stop(void);
void SendByte(uint8_t data);
uint8_t ReceiveAck(void);
void SendData(uint8_t data);

#endif
