// -*- mode:c;coding:utf-8 -*-
//****************************************************************************
//FILE:               sticks.c
//LANGUAGE:           c
//SYSTEM:             POSIX
//USER-INTERFACE:     NONE
//DESCRIPTION
//
//    Calcule la construction de baton la moins coûteuse.
//
//    https://programmingpraxis.com/2016/10/07/sticks/
//
//    On donne un paquet de batons, chacun d'une longueur entière.  Deux
//    batons peuvent être combiné en un seul baton plus long, par un
//    processus qui coûte la somme des longueurs des deux batons.  Votre but
//    est construire un seul baton, en combinant tous les batons originaux,
//    à un coût minimal.
//
//AUTHORS
//    <PJB> Pascal J. Bourguignon <pjb@informatimago.com>
//MODIFICATIONS
//    2016-10-10 <PJB> Created.
//BUGS
//LEGAL
//    AGPL3
//
//    Copyright Pascal J. Bourguignon 2016 - 2016
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//****************************************************************************

/*
*   Sticks
*   October 7, 2016
*
*   We continue our occasional series of textbook exercises:
*
*       You are given a bunch of sticks, each of integer length. Two
*       sticks can be combined into a single, larger stick by a process
*       that costs the sum of the lengths of the two sticks. Your goal is
*       to build a single stick combining all the original sticks, at
*       minimal cost.
*
*       For example, suppose you initially have three sticks of lengths 1,
*       2, and 4. You could combine sticks 2 and 4 at a cost of 6, then
*       combine that stick with stick 1 at a cost of 7, for a total cost
*       of 13. But it is better to first combine sticks 1 and 2 at a cost
*       of 3, then combine that stick with stick 4 at a cost of 7, for a
*       total cost of 10.
*
*   Your task is to write a program that combines a bunch of sticks at
*   minimal cost. When you are finished, you are welcome to read or run a
*   suggested solution, or to post your own solution or discuss the
*   exercise in the comments below.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <libgen.h>
#include <stdarg.h>


__attribute__ ((__noreturn__))
void error(const int status,const char* message,...){
    // Format and print an error message, and exit with the given exitCode.
    fprintf(stderr,"ERROR: ");
    va_list arguments;
    va_start(arguments,message);
    vfprintf(stderr,message,arguments);
    va_end(arguments);
    fprintf(stderr,"\n");
    exit((status==0)?EX_SOFTWARE:status);
}

char* string_copy(const char* string){
    char* copy=malloc(1+strlen(string));
    if(copy==0){
        error(EX_OSERR,"Out of memory");
    }
    strcpy(copy,string);
    return copy;
}

typedef unsigned int vector_element_t;
#define VECTOR_ELEMENT_FORMAT "%u"

typedef struct{
    unsigned int length;
    unsigned int allocated;
    vector_element_t* elements;
}   vector_t;

vector_t* vector_new(unsigned int allocated_size){
    vector_t* vector=malloc(sizeof(*vector));
    if(!vector){
        error(EX_UNAVAILABLE,"Out of Memory");
    }
    vector->elements=malloc(allocated_size*sizeof(vector->elements[0]));
    if(!vector->elements){
        error(EX_UNAVAILABLE,"Out of Memory");
    }
    vector->allocated=allocated_size;
    vector->length=0;
    return vector;
}

vector_element_t* vector_elements(vector_t* vector){
    return vector->elements;
}

unsigned int vector_length(vector_t* vector){
    return vector->length;
}

vector_t* vector_push(vector_t* vector,vector_element_t newElement){
    if(vector->length<vector->allocated){
        vector->elements[vector->length]=newElement;
        ++(vector->length);
        return vector;
    }else{
        vector_t* newVector=vector_new((vector->allocated+1)*2);
        memcpy(newVector->elements,vector->elements,sizeof(vector->elements[0])*vector->length);
        free(vector->elements);
        vector->elements=newVector->elements;
        free(newVector);
        return vector_push(vector,newElement);
    }
}


void vector_print(vector_t* vector,FILE* output,unsigned int start,unsigned int end){
    // prints the subrange of the vector from start (inclusive) to end (exclusive).
    // PRE: 0<=start<=end<=vector_length(vector);
    unsigned int i;
    vector_element_t* elements=vector_elements(vector);
    for(i=start;i<end;++i){
        fprintf(output,VECTOR_ELEMENT_FORMAT " ",elements[i]);
    }
}

int compare_vector_elements(const void* a,const void* b){
    vector_element_t ae=*(vector_element_t*)a;
    vector_element_t be=*(vector_element_t*)b;
    if(ae==be){
        return 0;
    }else if(ae<be){
        return -1;
    }else{
        return 1;
    }
}

vector_t* sticks(vector_t* stickLengths,FILE* output,int verbose){
    /*
    The smallest build cost will be obtained by pasting always the two
    smallest sticks.  Therefore we start by sorting the stick lengths,
    and then using the first two smallest.  We insert in order the new
    stick, until there remains only one.
    */

    qsort(vector_elements(stickLengths),vector_length(stickLengths),
          sizeof(vector_element_t),compare_vector_elements);

    if(verbose){
        fprintf(output,"Sorted sticks: " );
        vector_print(stickLengths,output,0,vector_length(stickLengths));
        fprintf(output,"\n");
    }

    vector_element_t* elements=vector_elements(stickLengths);
    unsigned int length=vector_length(stickLengths);
    unsigned int i=0;
    unsigned int cost=0;
    // This loop will process small sticks from the left to the right.
    // i is the index of the first remaining element.
    // The remaining sticks go from i to length-1.
    while(i+1<length){
        // build the new stick:
        unsigned int newStick=elements[i]+elements[i+1];
        cost+=newStick;
        if(verbose){
            fprintf(output,"Paste sticks " VECTOR_ELEMENT_FORMAT
                    " and " VECTOR_ELEMENT_FORMAT
                    " giving " VECTOR_ELEMENT_FORMAT
                    " at cost %u, total cost %u\n",
                    elements[i],elements[i+1],newStick,newStick,cost);
        }
        // We have one less element (two old sticks removed, one newStick to add):
        ++i;
        // insert the newStick in order, by moving the smaller elements down.
        unsigned int j=i+1;
        while((j<length)&&(elements[j+1]<newStick)){
            elements[j]=elements[j+1];
            ++j;
        }
        elements[j-1]=newStick;
        if(verbose){
            fprintf(output,"Remaining sticks: " );
            vector_print(stickLengths,output,i,length);
            fprintf(output,"\n");
        }
    }
    if(verbose){
        fprintf(output,"Total stick length: " VECTOR_ELEMENT_FORMAT "\n", elements[length-1]);
        fprintf(output,"Construction cost: %u\n",cost);
    }
    vector_t* result=vector_new(2);
    vector_push(result,elements[length-1]);
    vector_push(result,cost);
    return result;
}


void usage(const char* pname){
    fprintf(stderr,"\n%s usage:\n\n",pname);
    fprintf(stderr,"    %s [-v|--verbose] stickLength1 … stickLengthN\n\n",pname);
}

int main(const int argc,const char* argv[]){
    if(argc==1){
        usage(basename(string_copy(argv[0])));
        exit(EX_USAGE);
    }
    // Convert the arguments into a vector of ints:
    int verbose=0;
    vector_t* stickLengths=vector_new(argc);
    int i;
    for(i=1;i<argc;++i){
        if((strcmp(argv[i],"-v")==0)||(strcmp(argv[i],"--verbose")==0)){
            verbose=1;
        }else{
            vector_push(stickLengths,(unsigned int)atol(argv[i]));
        }
    }
    // Apply the sticks algorithm:
    vector_t* result=sticks(stickLengths,stdout,verbose);
    printf("Total length = %4u\n",vector_elements(result)[0]);
    printf("Total cost   = %4u\n",vector_elements(result)[1]);
    return(0);
}
