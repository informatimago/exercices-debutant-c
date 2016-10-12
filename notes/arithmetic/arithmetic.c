#include <limits.h>
#include <stdio.h>

int main(){
    printf("\n");
    printf("unsigned, standard, modulo 2^n arithmetic:\n");
    unsigned int ua=UINT_MAX;
    unsigned int ub=UINT_MAX;
    unsigned int uc=ua+ub;
    printf("uc=%u+%u=%u\n",ua,ub,uc);
    printf("\n");
    printf("signed, implementation dependant, anything can happen:\n");
    int a=INT_MAX;
    int b=INT_MAX;
    int c=a+b;
    printf("c=%d+%d=%d\n",a,b,c);
    printf("\n");
    return 0;
}
