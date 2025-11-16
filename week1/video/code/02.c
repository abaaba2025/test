#include <stdio.h>

int main()
{
    int i;

    printf("please input -1 or 0 or 1.");

    while(1)
    {
        scanf("%d",&i);

        if ( i == -1 ){
            break;
        }

        if ( i == 0 ){
            printf("helloworld\n");
        }

        if ( i == 1 ){
            printf("HELLOWORLD\n");
        }
    }

    return 0;
}