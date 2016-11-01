#include <stdio.h>
int main(){
    int n;
    do{
        printf("\nEncore un nombre: ");
        fflush(stdout);
        scanf("%20i",&n);
    }while(n);
    return 0;
}
