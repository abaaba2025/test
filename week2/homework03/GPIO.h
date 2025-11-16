#ifndef __GPIO_H
#define __GPIO_H

typedef enum{
    GPIO_Speed_2MHz,GPIO_Speed_10MHz,GPIO_Speed_50MHz
}GPIO_Speed_Type;

typedef struct{
    GPIO_Speed_Type GPIO_Speed;
}GPIO_InitTypeDef;

void GPIO_Init(GPIO_InitTypeDef *GPIO_StructureInit);

#endif