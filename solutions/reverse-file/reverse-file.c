// -*- mode:c;coding:utf-8 -*-
//****************************************************************************
//FILE:               reverse-file.c
//LANGUAGE:           c
//SYSTEM:             POSIX
//USER-INTERFACE:     NONE
//DESCRIPTION
//
//    Filtre inversant l'ordre des lignes.
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
#include <string.h>
#include <stdlib.h>
#include <sysexits.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>


__attribute__ ((__noreturn__))
void error(int exitCode,const char* formatControl,...){
    va_list args;
    fprintf(stderr,"ERROR: ");
    va_start(args,formatControl);
    vfprintf(stderr,formatControl,args);
    va_end(args);
    fprintf(stderr,"\n");
    exit((exitCode==0)?EX_SOFTWARE:exitCode);
}

void* check_not_null(void* address,int status,const char* formatControl,...){
    if(!address){
        char buffer[8192];
        va_list arguments;
        va_start(arguments,formatControl);
        vsnprintf(buffer,sizeof(buffer),formatControl,arguments);
        va_end(arguments);
        error(status,"%s",buffer);
    }
    return address;
}


void check_error_with_errno(int result,int status,const char* formatControl,...){
    if(result!=0){
        int saved_errno=errno;
        char buffer[8192];
        va_list arguments;
        va_start(arguments,formatControl);
        vsnprintf(buffer,sizeof(buffer),formatControl,arguments);
        va_end(arguments);
        error(status,"%s while %s",strerror(saved_errno),buffer);
    }
}

#ifndef TEST
enum{ big_buffer_size=512L*1024L*1024L };
const int max_line_length=4095;
#else
enum{ big_buffer_size=512L };
const int max_line_length=128;
#endif

static char big_buffer[big_buffer_size];
static int big_buffer_length;
static long long total_size;

typedef enum {
    status_read_line,   // one line is read
    status_eof,         // end-of-file is reached.
    status_buffer_full, // the big buffer is full.
} status_t;


status_t read_one_line(FILE* input,long long lino){
    // Read one line at the end of the big_buffer.
    int rest=big_buffer_size-big_buffer_length;
    if(rest<max_line_length){
        return status_buffer_full;
    }else{
        char* line=big_buffer+big_buffer_length;
        clearerr(input);
        char* new_line=fgets(line,max_line_length+1,input);
        if(new_line){
            int length=strlen(line);
            if(line[length-1]!='\n'){
                // The line is longer than max_line_length.
                error(EX_DATAERR,"Line no. %ll is too long.",lino);
            }
            big_buffer_length+=length;
            return status_read_line;
        }else{
            // error or eof.
            if(ferror(input)){
                error(EX_IOERR,"I/O error while reading input.");
            }
            // eof
            return status_eof;
        }
    }
}


status_t fill_big_buffer(FILE* input,long long* lino){
    // Fill the big buffer with lines read from input.
    big_buffer_length=0;
    status_t status=read_one_line(input,++(*lino));
    while(status==status_read_line){
        status=read_one_line(input,++(*lino));
    }
    return status;
}


void reverse_big_buffer(FILE* output){
    // The big_buffer is full from 0 to big_buffer_length
    // Scan the \n from the end and write out each line.
    // Warning: the buffer is modified by inserting nulls after each newline!
    char* end=big_buffer+big_buffer_length;
    char* start=end-1;
    while(big_buffer<start){
        while((big_buffer<start)&&(start[-1]!='\n')){
            --start;
        }
        *end=0;
        fputs(start,output);
        end=start;
        --start;
    }
}

static long unique=0; // a random number unique for each execution.

const char* generate_buffer_path(int index){
    // We build a pathname in $TMPDIR or /tmp by concatenating the
    // PID, the unique random number, and the file index.  Given the
    // same index in the same process, we should return an equal path.
    char* tmpdir=getenv("TMPDIR");
    if(tmpdir==0){
        tmpdir="/tmp";
    }
    int size=(strlen(tmpdir)+1+10+1+8+1+3+5+1+16);
    char* buffer=0;
try_again:
    buffer=check_not_null(malloc(size),
                          EX_OSERR,"Out of Memory");
    int would_size=snprintf(buffer,size,"%s/%010d-%08lx-%03d.data",
                            tmpdir,getpid(),unique,index);
    if(would_size>size){
        free(buffer);
        size=would_size+1;
        goto try_again;
    }
    return buffer;
}

size_t fsize(FILE* file){
    clearerr(file);
    ssize_t current_position=ftell(file);
    if(current_position<0){
        error(EX_IOERR,"ftell: %s",strerror(errno));
    }
    check_error_with_errno(fseek(file,0,SEEK_END),
                           EX_IOERR,"fseek");
    size_t size=(size_t)ftell(file);
    check_error_with_errno(fseek(file,current_position,SEEK_SET),
                           EX_IOERR,"fseek");
    return size;
}

void save_big_buffer(int index,char buffer[],int size){
    const char* path=generate_buffer_path(index);
    FILE* file=check_not_null(fopen(path,"w"),
                              EX_UNAVAILABLE,"Creating buffer file %s",path);
    size_t written=fwrite(buffer,sizeof(char),size,file);
    if(written!=(unsigned long)size){
        error(EX_UNAVAILABLE,"Writing buffer file %s (written=%d instead of %d)",
              path,written,size);
    }
    fclose(file);
    free((void*)path);
}


int load_big_buffer(int index,char buffer[]){
    const char* path=generate_buffer_path(index);
    FILE* file=check_not_null(fopen(path,"r"),
                              EX_UNAVAILABLE,"Opening buffer file %s",path);
    size_t size=fsize(file);
    size_t read_bytes=fread(buffer,sizeof(char),size,file);
    if(read_bytes!=size){
        error(EX_UNAVAILABLE,"Reading buffer file %s (read=%d instead of %d)",
              path,read_bytes,size);
    }
    fclose(file);
    check_error_with_errno(remove(path),EX_OSERR,"removing %s",path);
    free((void*)path);
    return (int)read_bytes;
}

void delete_big_buffer(int index){
    const char* path=generate_buffer_path(index);
    check_error_with_errno(remove(path),EX_OSERR,"removing %s",path);
    free((void*)path);
}


void reverse_big_file(FILE* input,FILE* output,long long lino){
    // big_buffer is full.
    // Save the big_buffer to a file, and go on reading and saving until the end of file.
    // Then reverse everything.
    int buffer_count=0;
    status_t status=status_buffer_full;
    while(status==status_buffer_full){
        total_size+=big_buffer_length;
        if(total_size>64LL*1024LL*1024LL*1024LL){
            while(buffer_count>0){
                delete_big_buffer(--buffer_count);
            }
            error(EX_USAGE,"Input stream bigger than 64 GB.");
        }
        save_big_buffer(buffer_count++,big_buffer,big_buffer_length);
        status=fill_big_buffer(input,&lino);
    }
    // 1- reverse the current big_buffer
    reverse_big_buffer(output);
    // 2- reverse the big_buffer stored in files, in reverse order:
    while(buffer_count>0){
        big_buffer_length=load_big_buffer(--buffer_count,big_buffer);
        reverse_big_buffer(output);
    }
}


void reverse_file(FILE* input,FILE* output){
    total_size=0LL;
    long long lino=0LL;
    status_t status=fill_big_buffer(input,&lino);
    switch(status){
      case status_eof:
          reverse_big_buffer(output);
          break;
      case status_buffer_full:
          reverse_big_file(input,output,lino);
          break;
      default: // never occurs.
          break;
    }
}

void initialize(){
    srandom(clock());
    unique=random();
}


int main(){
    initialize();
    reverse_file(stdin,stdout);
    return 0;
}
