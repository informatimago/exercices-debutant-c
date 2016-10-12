// -*- mode:c;coding:utf-8 -*-
//****************************************************************************
//FILE:               convert-from-fixed-to-csv.c
//LANGUAGE:           c
//SYSTEM:             POSIX
//USER-INTERFACE:     NONE
//DESCRIPTION
//    
//    Converti un fichier d'enregistremet fixe vers le format CSV.
//    
//AUTHORS
//    <PJB> Pascal J. Bourguignon <pjb@informatimago.com>
//MODIFICATIONS
//    2016-10-12 <PJB> Created.
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

#include <limits.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sysexits.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>

#define unused __attribute__((unused))

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


char* string_copy(const char* string){
    char* copy=malloc(1+strlen(string));
    if(copy==0){
        error(EX_OSERR,"Out of memory");
    }
    strcpy(copy,string);
    return copy;
}

int strcnt(const char string[],char character){
    // return the number of ocurences of character in string.
    int i=0;
    int count=0;
    while(string[i]!=0){
        if(string[i]==character){
            ++count;
        }
        ++i;
    }
    return count;
}




/*

On va représenter dans des structures de données, la description des
champs et des enregistrement, ainsi que les valeurs de champs d'un
enregistrement.  

Comme on identifie seulement trois type de champs: entier, nombre à
virguel fixe (entier avec un virgule virtuelle) et chaîne, ça
simplifie le programme puisqu'on va utiliser une même fonction pour
chaque opération sur chaque champ.

*/

typedef enum {
    pic_entier,
    pic_fixed,
    pic_string
} pic_t;

typedef struct {
    const char* name;
    pic_t picture;
    int width;
    int virtual_comma;
} field_description_t;

// description of the input records
field_description_t vendeur[]={
            {"magasin", pic_entier, 3,  0}, 
            {"novend",  pic_entier, 4,  0}, 
            {"nom",     pic_string, 20, 0}, 
            {"prenom",  pic_string, 15, 0},
            {"mont1",   pic_fixed,  6,  2}, 
            {"mont2",   pic_fixed,  6,  2}, 
            {"mont3",   pic_fixed,  6,  2}, 
            {"prim1",   pic_fixed,  5,  2}, 
            {"prim2",   pic_fixed,  5,  2}, 
            {"prim3",   pic_fixed,  5,  2}, 
            {"filler",  pic_string, 6,  0}, 
        };

#define countof(vector) (sizeof((vector))/sizeof((vector)[0]))


// structure pour stocker la valeur des champs.
typedef struct {
    pic_t picture;
    int width;
    union {
        struct {
            unsigned long value;
        } entier;
        struct {
            unsigned long value;
            int virtual_comma;
        } fixed;
        struct {
            char* value;
        } string;
    } info;
} field_value_t;


void parse_field(field_description_t* field,field_value_t* value,
                 const char record[]){
    // parses a single field, storing the value
    switch(field->picture){
      case pic_entier:
          // fall thru.
      case pic_fixed:{
          char format[8];
          sprintf(format,"%%%dlu",field->width);
          unsigned long field_value;
          sscanf(record,format,&field_value);
          value->picture=field->picture;
          value->width=field->width;
          if(field->picture==pic_entier){
              value->info.entier.value=field_value;
          }else{
              value->info.fixed.value=field_value;
              value->info.fixed.virtual_comma=field->virtual_comma;
          }
          break;
      }
      case pic_string:{
          char* string=check_not_null(malloc(field->width+1),EX_OSERR,"Out of Memory");
          strncpy(string,record,field->width);
          // trim trailling spaces:
          int i=field->width;
          while((i>0)&&(string[i-1]==' ')){
              --i;
          }
          string[i]=0;
          value->picture=field->picture;
          value->width=field->width;
          value->info.string.value=string;
          break;
      }
      default:
          error(EX_SOFTWARE,"Unexpected field picture (field name: %s, picture: %d)",
                field->name,field->picture);
    }
        
}

void parse_record(int nfields,field_description_t fields[],field_value_t values[],
                  const char record[]){
    // parses the record for the fields, storing the values.
    int i;
    int position=0;
    for(i=0;i<nfields;++i){
        parse_field(&fields[i],&values[i],record+position);
        position+=fields[i].width;
    }
}


const int ENTIER_MAX_FIELD_SIZE=24; // (+ 2 (ceiling (log (expt 2 ULONG_MAX) 10)))

char* generate_csv_field_for_value(field_value_t* value){
    switch(value->picture){
      case pic_entier:{
          char* buffer=check_not_null(malloc(ENTIER_MAX_FIELD_SIZE),EX_OSERR,"Out of Memory");
          sprintf(buffer,"%lu",value->info.entier.value);
          return buffer;
      }          
      case pic_fixed:{
          assert(value->info.fixed.virtual_comma<value->width);
          assert(0<value->info.fixed.virtual_comma);
          char* buffer=check_not_null(malloc(ENTIER_MAX_FIELD_SIZE),EX_OSERR,"Out of Memory");
          sprintf(buffer,"%*lu",1+value->info.fixed.virtual_comma,value->info.fixed.value);
          // now, we shift the decimal one character to the right, to leave space for the dot.
          int comma=value->info.fixed.virtual_comma;
          int p=strlen(buffer);
          buffer[p+1]=0;
          while(comma>0){
              --comma;
              buffer[p]=buffer[p-1];
              --p;
          }
          buffer[p]='.';
          // now, we replace spaces with zeros, if needed.
          // The initial assert ensures that p>1.
          if(buffer[p-1]==' '){
              buffer[p-1]='0';
          }
          ++p;
          while(buffer[p]==' '){
              buffer[p]='0';
              ++p;
          }
          return buffer;
      }
      case pic_string:{
          const char* string=value->info.string.value;
          int len=strlen(string);
          while((len>0)&&(string[len-1]==' ')){ // trim trailing spaces
              --len;
          }
          int quotes=strcnt(string,'"');
          char* buffer=check_not_null(malloc(len+quotes+3),EX_OSERR,"Out of Memory");
          int src=0;
          int dst=0;
          buffer[dst++]='"';
          while(len>0){
              --len;
              if(string[src]=='"'){
                  buffer[dst++]='"';
              }
              buffer[dst++]=string[src++];
          }
          buffer[dst++]='"';
          buffer[dst]=0;
          return buffer;
      }
      default:
          return "";
    }
}

const int RECORD_SIZE=81;

void convert(FILE* input,FILE* output){
    unsigned long read_count=0;
    unsigned long written_count=0;
    field_value_t record[countof(vendeur)];
    char buffer[128];
    while(fgets(buffer,RECORD_SIZE+1,input)){
        ++read_count;
        parse_record(countof(vendeur),vendeur,record,buffer);
        const char* sep="";
        unsigned int i;
        for(i=0;i<countof(vendeur);++i){
            if(strcmp("filler",vendeur[i].name)!=0){
                fprintf(output,"%s%s",sep,generate_csv_field_for_value(&record[i]));
                sep=",";
            }
        }
        fprintf(output,"\n");
        ++written_count;
    }
    fprintf(stderr,"Read %lu lines\nWritten %lu lines\n",read_count,written_count);
    if(!feof(input)){
        error(EX_IOERR,"Reading input file");
    }
}



// Option processing

const char* pname;
FILE* input;
FILE* output;

struct option;
void usage(unused struct option* opt,unused const char* argument);

void open_input(unused struct option* opt,const char* path){
    input=check_not_null(fopen(path,"r"),EX_DATAERR,"Cannot open input file %s.",path);
}

void open_output(unused struct option* opt,const char* path){
    output=check_not_null(fopen(path,"w"),EX_DATAERR,"Cannot open output file %s.",path);
}


typedef struct option {
    const char* short_option;
    const char* long_option;
    const char* argument_name;
    void (*process_option)(struct option*,const char*);
} option_t;

option_t options[]={
        {"-h", "--help",   0,                 &usage},
        {"-i", "--input",  "$inputFilePath",  &open_input},
        {"-o", "--output", "$outputFilePath", &open_output},
};

void usage(unused struct option* opt,unused const char* argument){
    fprintf(stderr,"%s usage\n\n",pname);
    fprintf(stderr,"    %s ",pname);
    unsigned int i=0;
    for(i=0;i<countof(options);++i){
        fprintf(stderr,"[%s|%s",
                options[i].short_option,
                options[i].long_option);
        if(options[i].argument_name){
            fprintf(stderr," %s] ",
                    options[i].argument_name);
        }else{
            fprintf(stderr,"] ");
        }
    }
    fprintf(stderr,"\n\n");
}


void process_arguments(const int argc,const char* argv[]){
    unsigned int j;
    int i=1;
    while(i<argc){
        for(j=0;j<countof(options);++j){
            if((strcmp(options[j].short_option,argv[i])==0)||
               (strcmp(options[j].long_option,argv[i])==0)){
                if(options[j].argument_name){
                    if(i+1<argc){
                        ++i;
                        options[j].process_option(&options[j],argv[i]);
                    }
                }else{
                    error(EX_USAGE,"Missing %s after %s",
                          options[j].argument_name,argv[i]);
                }
                break;
            }
        }
        ++i;
    }
}

int main(const int argc,const char* argv[]){
    pname=basename(string_copy(argv[0]));
    input=stdin;
    output=stdout;
    process_arguments(argc,argv);
    convert(input,output);
    fclose(output);
    return 0;
}

