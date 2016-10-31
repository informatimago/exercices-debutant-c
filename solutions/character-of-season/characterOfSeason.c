// -*- mode:c;coding:utf-8 -*-
//****************************************************************************
//FILE:               characterOfSeason.c
//LANGUAGE:           c
//SYSTEM:             POSIX
//USER-INTERFACE:     NONE
//DESCRIPTION
//
//    Imprime un caractère séléctionné en fonction de la saison
//    de la date indiquée.
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
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sysexits.h>
#include <sys/time.h>

/*
    | index:     | 0 | 1 | 2 |  3 |  4 |                        |
    | character: | * | & | @ |  % |  * |                        |
    |------------+---+---+---+----+----+------------------------|
    | mois:      | 1 | 4 | 7 | 10 |    | index = (mois-1)/3     |
    |            | 2 | 5 | 8 | 11 |    | index = (mois-1)/3     |
    | day<21     | 3 | 6 | 9 | 12 |    | index = (mois-1)/3     |
    | day>=21    |   | 3 | 6 |  9 | 12 | index = (mois-1)/3 + 1 |
    |            |   |   |   |    |    |                        |
 */

char characterForSeason(const int month,const int day){
  static const char seasonCharacter[]="*&@%*";
  return seasonCharacter[((month-1)/3)+(((month%3)==0)
                                        ?((day<21)?0:1)
                                        :0)];
}

void assertEqual(const char result,const char expected,const char* expression){
  if(result!=expected){
    fprintf(stderr,"FAILURE: %s\n",expression);
    fprintf(stderr,"  returned: %c\n",result);
    fprintf(stderr,"  expected: %c\n",expected);
  }
}

void test_characterForSeason(){
  assertEqual(characterForSeason(1, 1), '*',"characterForSeason  1  1");
  assertEqual(characterForSeason(3,10), '*',"characterForSeason  3 10");
  assertEqual(characterForSeason(3,30), '&',"characterForSeason  3 30");
  assertEqual(characterForSeason(8,10), '@',"characterForSeason  8 10");
  assertEqual(characterForSeason(8,30), '@',"characterForSeason  8 30");
  assertEqual(characterForSeason(9,22), '%',"characterForSeason  9 22");
  assertEqual(characterForSeason(12, 1),'%',"characterForSeason 12  1");
  assertEqual(characterForSeason(12,31),'*',"characterForSeason 12 31");
}

char characterForSeason_switch(const int month,const int day){
  switch(month){
  case  1: case  2: return '*';
  case  3:          return (day<21)?'*':'&';
  case  4: case  5: return '&';
  case  6:          return (day<21)?'&':'@';
  case  7: case  8: return '@';
  case  9:          return (day<21)?'@':'%';
  case 10: case 11: return '%';
  case 12:          return (day<21)?'%':'*';
  }
  return '#'; /* invalid input date */
}

void test_characterForSeason_switch(){
  assertEqual(characterForSeason_switch(1, 1), '*',"characterForSeason_switch  1  1");
  assertEqual(characterForSeason_switch(3,10), '*',"characterForSeason_switch  3 10");
  assertEqual(characterForSeason_switch(3,30), '&',"characterForSeason_switch  3 30");
  assertEqual(characterForSeason_switch(8,10), '@',"characterForSeason_switch  8 10");
  assertEqual(characterForSeason_switch(8,30), '@',"characterForSeason_switch  8 30");
  assertEqual(characterForSeason_switch(9,22), '%',"characterForSeason_switch  9 22");
  assertEqual(characterForSeason_switch(12, 1),'%',"characterForSeason_switch 12  1");
  assertEqual(characterForSeason_switch(12,31),'*',"characterForSeason_switch 12 31");
}

int main(const int argc,const char* argv[]){
  test_characterForSeason();
  test_characterForSeason_switch();
  int month=1;
  int day=1;
  if(argc==3){
      month=atoi(argv[1]);
      day=atoi(argv[2]);
  }else{
      struct timeval time;
      int res=gettimeofday(&time,0);
      if(res!=0){
          perror("gettimeofday");
          exit(EX_UNAVAILABLE);
      }
      struct tm* date=localtime(&time.tv_sec);
      month=date->tm_mon+1;
      day=date->tm_mday;
  }
  char cos=characterForSeason(month,day);
  printf("%c\n",cos);
  return 0;
}
