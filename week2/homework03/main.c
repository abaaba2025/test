#include "GPIO.h"

int main (void)
{
    GPIO_InitTypeDef GPIO_StructureInit;

    GPIO_Init(&GPIO_StructureInit);

    return 0;
}