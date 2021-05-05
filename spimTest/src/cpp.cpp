#include <stdio.h>

int main(){
    __int8_t num = 0x95;
    printf("num is %d\n",num);
    __int8_t num2 = num >> 4;
    printf("num2 is %d\n",num2);
    int num3 = (int)num << 8;
    printf("num3 is %d\n",num3);
    __int8_t num4 = (__int8_t)num;
    printf("num4 is %d\n",num4);
    return 0;
}