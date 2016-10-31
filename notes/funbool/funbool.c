// -*- mode:c;coding:utf-8 -*-
//****************************************************************************
//FILE:               funbool.c
//LANGUAGE:           c
//SYSTEM:             POSIX
//USER-INTERFACE:     NONE
//DESCRIPTION
//
//    Booleans implemented with Functions.
//
//AUTHORS
//    <PJB> Pascal J. Bourguignon <pjb@informatimago.com>
//MODIFICATIONS
//    2016-10-16 <PJB> Created.
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

#include <stdio.h>

#define unused __attribute__((unused))
#define countof(v) (sizeof(v)/sizeof((v)[0]))


/*
Here, we implement booleans and boole operators using function.
True and false are two functions taking two arguments: true()
returns the first argument, false() returns the second argument.
*/

typedef int (*booleen)(int,int);
int true(int a,unused int b){return a;}
int false(unused int a,int b){return b;}

/*

We can therefore implement IF rather easily, but just calling the
boolean function with the then and else values.

Notice: since C is not a lazy programming language, the arguments are
evaluated BEFORE calling the function, and therefore both the then and
the else expressions are evaluated before calling the condition
function! This is very different from the ternary if:
(condition?then:else), where the then or the else is not evaluated
when it doesn't need to.
*/
#define IF(condition,then,else) (condition(then,else))

/* Then, we can implement AND and OR like this: */
#define u(x) x ## __LINE__
#define NOT(a)    ({booleen r[]={&true,&false}; IF(a,1,0)[r];}) // NOTE: This works only with gcc!
#define AND(a,b)  ({booleen r[]={b,&false}; IF(a,0,1)[r];})     // NOTE: This works only with gcc!
#define OR(a,b)   ({booleen r[]={&true, b}; IF(a,0,1)[r];})     // NOTE: This works only with gcc!


/* Demonstration */
int example(){

    const char* bs[]={"faux","vrai"};

    // Let's define a and b as two tables:
    booleen a[]={&true,&true,&false,&false};
    booleen b[]={&true,&false,&true,&false};
    int     x[]={1,2,3,4};
    int     y[]={101,202,303,404};
    unsigned int i;
    // We compute the truth table of both operator, and additionnaly,
    // use obtained functions to select between two integers.
    printf("NON\n");
    for(i=0;i<2;++i){
        printf("NON %s = %s\n",
               bs[b[i](1,0)],                    // label for b[i]
               bs[NOT(b[i])(1,0)]);              // label for NON(b[i])
    }
    printf("ET\n");
    for(i=0;i<countof(a);++i){
        printf("%s /\\ %s = %s -> %d\n",
               bs[a[i](1,0)],                    // label for a[i]
               bs[b[i](1,0)],                    // label for b[i]
               bs[AND(a[i],b[i])(1,0)],          // label for AND(a[i],b[i])
               IF(AND(a[i],b[i]),x[i],y[i]));    // selection between x[i] and y[i]
    }
    printf("OU\n");
    for(i=0;i<countof(a);++i){
        printf("%s \\/ %s = %s -> %d\n",
               bs[a[i](1,0)],                    // label for a[i]
               bs[b[i](1,0)],                    // label for b[i]
               bs[OR(a[i],b[i])(1,0)],           // label for OR(a[i],b[i])
               IF(OR(a[i],b[i]),x[i],y[i]));     // selection between x[i] and y[i]
    }
    return 0;
}

int main(){
    example();
    return 0;
}
