// -*- mode:c;coding:utf-8 -*-
//****************************************************************************
//FILE:               reverse-lines-by-character.c
//LANGUAGE:           c
//SYSTEM:             POSIX
//USER-INTERFACE:     NONE
//DESCRIPTION
//
//    Filter inversant chaque ligne, caractère par caractère, en
//    tenant compte de l'encodage specifié par la variable
//    d'environnement LC_CTYPE ou à défaut, LC_ALL.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sysexits.h>
#include <errno.h>
#include <locale.h>


__attribute__ ((__noreturn__))
void error(int exitCode,const char* formatControl,...){
    // Format and print an error message, and exit with the given exitCode.
    va_list args;
    fprintf(stderr,"ERROR: ");
    va_start(args,formatControl);
    vfprintf(stderr,formatControl,args);
    va_end(args);
    fprintf(stderr,"\n");
    exit((exitCode==0)?EX_SOFTWARE:exitCode);
}


const int buffer_size=4096; // maximum line length in byte.

void wcs_reverse(wchar_t buffer[],size_t size){
    // reverse the order of the the wide characters in the buffer.
    if(size>0){
        size_t i=0;
        size_t j=size-1;
        while(i<j){
            wchar_t temp=buffer[i];
            buffer[i]=buffer[j];
            buffer[j]=temp;
            ++i;
            --j;
        }
    }
}

void reverse_one_line(char byte_buffer[],size_t byte_length,int lino){
    // decode the byte_byffer string, reverse its characters, and re-encode it.
    // On entry, byte_buffer doesn't contain the terminating newline.
    // on exit, a newline is added to the byte_buffer.
    wchar_t character_buffer[buffer_size];
    ssize_t character_length=mbstowcs(character_buffer,byte_buffer,buffer_size);
    if(character_length<0){
        error(EX_DATAERR,"Found invalid code sequence on line %d\n",lino);
    }
    wcs_reverse(character_buffer,character_length);
    character_buffer[character_length]=0;
    byte_length=wcstombs(byte_buffer,character_buffer,buffer_size-2);
    byte_buffer[byte_length]='\n';
    ++byte_length;
    byte_buffer[byte_length]=0;
}

typedef void(*process_buffer_pr)(char[],size_t,int);

void filter_line_by_line(FILE* input,FILE* output,process_buffer_pr process_one_line){
    // Cette fonction applique un filtre (process_one_line) ligne par ligne
    // sur les lignes lues du flôt input, et écrit le résultat sur le flôt output.
    // process_one_line prend un tampon contenant la ligne (sans le newline),
    // et modife ce tampon, en ajoutant un newline (ou non, si la ligne suivant
    // doit être concaténée).
    int     lino=0;
    char    byte_buffer[buffer_size];
    clearerr(input);
    while(fgets(byte_buffer,buffer_size,input)){
        ++lino;
        size_t byte_length=strlen(byte_buffer);
        if(byte_length>0){
            if(byte_buffer[byte_length-1]=='\n'){
                --byte_length;
                byte_buffer[byte_length]=0;
            }else{
                // Line was too long.
                error(EX_DATAERR,"Line %d is too long.",lino);
            }
        }
        process_one_line(byte_buffer,byte_length,lino);
        errno=0;
        int result=fputs(byte_buffer,output);
        if(result<0){
            error(EX_OSERR,strerror(errno));
        }
        clearerr(input);
    }
    int result=ferror(input);
    if(result!=0){
        error(EX_IOERR,"Error while reading input.");
    }
}

void reverse_lines(FILE* input,FILE* output){
    // Ce filtre inverse l'ordre des caractères de chaque ligne.
    filter_line_by_line(input,output,reverse_one_line);
}


int main(){
    setlocale(LC_ALL,getenv("LC_ALL"));
    setlocale(LC_CTYPE,getenv("LC_CTYPE"));
    reverse_lines(stdin,stdout);
    return 0;
}
