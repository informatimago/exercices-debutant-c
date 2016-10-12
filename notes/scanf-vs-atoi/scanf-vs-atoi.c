#include <stdio.h>
#include <stdlib.h>


int main(int argc,char* argv[]){
    int i;
    for(i=1;i<argc;i++){
        printf("atoi(\"%s\")                          -> %4d\n",argv[i],atoi(argv[i]));
        int value;
        int n=sscanf(argv[i],"%i",&value);
        printf("sscanf(\"%s\",\"%%i\",&value) -> %d, value = %4d\n",argv[i],n,value);
        printf("\n");
    }
    return 0;

}
