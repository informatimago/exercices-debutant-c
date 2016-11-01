#include <stdio.h>
#include <stdlib.h>

int main(){
    int impairs=0;
    int pairs=0;
    int* compteur=0;
    char buffer[80];
    while(fgets(buffer,sizeof(buffer),stdin)){
        int n=atoi(buffer);
        if(n&1){
            compteur=&impairs;
        }else{
            compteur=&pairs;
        }
        ++(*compteur);
    }
    printf("impairs : %d\n",impairs);
    printf("pairs   : %d\n",pairs);
    return 0;
}

/*
-*- mode: compilation; default-directory: "~/src/public/exercices/notes/pointeurs/" -*-
Compilation started at Tue Nov  1 00:55:05

make test
cc     pointeurs.c   -o pointeurs
printf "%d\n" 1 2 3 5 7 11 13 | ./pointeurs
impairs : 6
pairs   : 1

Compilation finished at Tue Nov  1 00:55:05
*/
