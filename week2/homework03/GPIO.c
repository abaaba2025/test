#include "GPIO.h"

void GPIO_Init(GPIO_InitTypeDef *GPIO_StructureInit)
{
    GPIO_StructureInit->GPIO_Speed = GPIO_Speed_2MHz;
}
