#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef int bool;
enum{false=0,true=1};


void error_out_of_bounds(){
    fprintf(stderr,"ERROR: vector reference out of bounds.\n");
    exit(1);
}
void error_out_of_memory(){
    fprintf(stderr,"ERROR: out of memory.\n");
    exit(1);
}

typedef struct {
    int* elements;
    int  length;
}  vector_t;

vector_t* vector_new(int length){
    vector_t* vector=malloc(sizeof(*vector));
    if(!vector){ error_out_of_memory(); }
    vector->length=length;
    vector->elements=malloc(sizeof(vector->elements[0])*length);
    if(!vector->elements){ error_out_of_memory(); }
    return vector;
}

extern inline int vector_length(vector_t* vector){
    return vector->length;
}

extern inline int vector_ref(vector_t* vector,int index){
    if((index<0)||(vector_length(vector)<=index)){
        error_out_of_bounds();
    }
    return vector->elements[index];
}

extern inline void vector_set(vector_t* vector,int index,int value){
    if((index<0)||(vector_length(vector)<=index)){
        error_out_of_bounds();
    }
    vector->elements[index]=value;
}

bool winnable_position_p(vector_t* jumps,int position){
    if(vector_length(jumps)<=position){
        return true;
    }
    int jumpable=vector_ref(jumps,position);
    if(jumpable==0){
        return false;
    }
    for(int jump=jumpable;1<=jump;--jump){
        if(winnable_position_p(jumps,position+jump)){
            return true;
        }
    }
    return false;
}

bool winnablep(vector_t* jumps){
    return winnable_position_p(jumps,0);
}



typedef struct cons {
    int element;
    struct cons* next;
} cons_t;

cons_t* cons(int element,cons_t* next){
    cons_t* cons=malloc(sizeof(*cons));
    if(!cons){ error_out_of_memory(); }
    cons->element=element;
    cons->next=next;
    return cons;
}

extern inline int cons_element(cons_t* cons){
    return cons->element;
}

extern inline cons_t* cons_next(cons_t* cons){
    return cons->next;
}

cons_t* winnable_path_from_position(vector_t* jumps,int position){
    if(vector_length(jumps)<=position){
        return cons(-1,0);
    }else{
        int jumpable=vector_ref(jumps,position);
        if(jumpable==0){
            return 0;
        }else{
            for(int jump=jumpable;1<=jump;--jump){
                cons_t* path=winnable_path_from_position(jumps,position+jump);
                if(path){
                    return cons(jump,path);
                }
            }
            return 0;
        }
    }
}

cons_t* winnable_path(vector_t* jumps){
    return winnable_path_from_position(jumps,0);
}


int main(int argc,char* argv[]){
    if(argc<=1){
        printf("Usage:\n\n\tsauts cell...\n\n");
        exit(1);
    }
    if(strcmp(argv[1],"--path")==0){
        vector_t* jumps=vector_new(argc-2);
        for(int i=2;i<argc;++i){
            vector_set(jumps,i-2,atoi(argv[i]));
        }
        cons_t* path=winnable_path(jumps);
        printf(" --> ");
        if(!path){
            printf("not winnable\n");
        }else{
            while(path){
                printf("%d ",cons_element(path));
                path=cons_next(path);
            }
            printf("\n");
        }
    }else{
        vector_t* jumps=vector_new(argc-1);
        for(int i=1;i<argc;++i){
            vector_set(jumps,i-1,atoi(argv[i]));
        }
        printf(" --> ");
        if(winnablep(jumps)){
            printf("winnable\n");
        }else{
            printf("not winnable\n");
        }
    }
    return 0;
}
