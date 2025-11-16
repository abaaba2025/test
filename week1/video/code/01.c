#include <stdio.h>

int main()
{
    int Identity;
    printf("If you aren't AI,please input '1'.");
    scanf("%d",&Identity);

    if ( Identity != 1 ){
        printf("YES,you are AI");
    }
    
    if ( Identity == 1 ){
        printf("NO,you are not AI");
    }

    return 0 ;
}