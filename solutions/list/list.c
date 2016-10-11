#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sysexits.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

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




// Commençons par définir une structure de liste chaînée:

typedef int list_element_t;
typedef struct list_node {
    list_element_t element;
    struct list_node* next;
} list_t;

// Voici les deux accesseurs pour lire les champs des noeuds de liste:

list_element_t element(list_t* list){
    return list->element;
}

list_t* rest(list_t* list){
    return list->next;
}

list_t* set_rest(list_t* list,list_t* newRest){
    return list->next=newRest;
}

// et voici le constructeur de noeud de liste:

list_t* cons(list_element_t element,list_t* next){
    list_t* new_node=check_not_null(malloc(sizeof(*new_node)),
                                    EX_OSERR,"Out of Memory");
    new_node->element=element;
    new_node->next=next;
    return new_node;
}


// affiche une list (format lisp):
void fprint_list(FILE* out,list_t* list){
    if(list){
        const char* sep="(";
        while(list){
            fprintf(out,"%s%d",sep,element(list));
            sep=" ";
            list=rest(list);
        }
        fprintf(out,")");
    }else{
        fprintf(out,"()");
    }
}
            

// Une fonction pour inverser sur place l'ordre des noeuds dans une liste:

list_t* reverse(list_t* list){
    list_t* result=0;
    while(list){
        list_t* next=rest(list);
        list->next=result;
        result=list;
        list=next;
    }
    return result;
}

// Une fonction pour construire facilement une nouvelle liste:
// list_t l=list(5,10,20,30,40,50); // == (10 20 30 40 50)
list_t* list(int count,list_element_t element,...){
    va_list arguments;
    list_t* result=0;
    switch(count){
      case 0:
          return result;
      case 1:
          return cons(element,0);
      default:
          result=cons(element,0);
          va_start(arguments,element);
          while(--count>0){
              list_element_t element=va_arg(arguments,list_element_t);
              result=cons(element,result);
          }
          va_end(arguments);
          return reverse(result);
    }
}


// Nous allons avoir à filtrer des listes selon différents critères
// (prédicats); voici une fonction de filtre paramétrée par une
// fonction prédicat:

typedef int (*predicate_f)(list_element_t,void* data);

list_t* filter(list_t* list,predicate_f predicate,void* data){
    list_t* result=0;
    while(list){
        if(!predicate(element(list),data)){
            result=cons(element(list),result);
        }
        list=rest(list);
    }
    return reverse(result);
}

#define unused __attribute__((unused))

// un prédicat pour les nombre impairs:
int oddp(list_element_t element,unused void* data){
    return element&1;
}

// un filtre pour enlever les éléments impairs:
list_t* remove_odd_elements(list_t* list){
    return filter(list,oddp,0);
}


// un prédicat pour indiquer tous les n-ième élements:
typedef struct {
    int i;
    int n;
} closure_every_nth_t;

int every_nth(unused list_element_t element,void* data){
    closure_every_nth_t* closure=data;
    closure->i++;
    if(closure->i>=closure->n){
        closure->i=0;
        return 1;
    }
    return 0;
}

// un filtre pour enlever les n-ièmes élements.
// Note: on fait une copie au lieu de couper la liste originale.
list_t* remove_nth_elements(list_t* list,int n){
    closure_every_nth_t closure={0,n};
    return filter(list,every_nth,&closure);
}

// une fonction pour trouver le millieu d'une liste: on utilise le
// truc du lapin et de la tortue, avec une variante.  Le lapin court
// deux fois plus vite que la tortue, quand il atteint la fin de la
// liste, la tortue est au millieu.  Cependant, nous avons besoin du
// dernier noeud de la première moitié, et du dernier noeud de la
// seconde moitié.  Ainsi nous alons tester rest(tortue) et
// rest(lapin) au lieu de tortue et lapin.


void millieu(list_t** vtortue,list_t** vlapin){
    // We want the middle node to belong to the second half on odd-length,
    // so we add a node to the turtle to make it be late.
    list_t* tortue=cons(0,(*vtortue));
    list_t* tete_tortue=tortue;
    list_t* lapin=(*vlapin);
    assert(lapin);
    while(lapin&&rest(lapin)&&rest(rest(lapin))){
        tortue=rest(tortue);
        lapin=rest(rest(lapin));
    }
    if(rest(lapin)==0){
        // lapin a parcouru N=L=2T elements.
        (*vtortue)=tortue;
        (*vlapin)=lapin;
    }else{
        // lapin a parcouru N-1=L=2T; N=T+(T+1)
        (*vtortue)=rest(tortue);
        (*vlapin)=rest(lapin);
    }
    free(tete_tortue);
}


list_t* swap_halves(list_t* list){
    if((!list)||(!rest(list))){
        // 0 ou 1 élement.
        return list;
    }

    // pour couper la liste elle-même, on a besoin que tortue indique
    // le dernier élement de la première moitié.  On lui ajoute alors un élement:
    list_t* tortue=list;
    list_t* lapin=list;
    millieu(&tortue,&lapin);
    //   list              tortue   result       lapin
    //    |                  |        |            |
    //    V                  V        V            V
    //  [H1|*]--> …        [L1|*]-->[H2|*]--> …  [L2|0]
    //    première moitié         X seconde moitié

    list_t* result=rest(tortue);
    set_rest(lapin,list);
    set_rest(tortue,0);
    return result;
}



int main(){
    
    list_t* l10=list(10,0,1,2,3,4,5,6,7,8,9);
    fprint_list(stdout,l10);
    fprintf(stdout,"\nwith odd number removed: ");
    list_t* even=remove_odd_elements(l10);
    fprint_list(stdout,even);
    fprintf(stdout,"\n\n");

    list_t* l20=list(20,
                    0,1,2,3,4,5,6,7,8,9,
                    10,11,12,13,14,15,16,17,18,19);
    fprint_list(stdout,l20);
    fprintf(stdout,"\nwith every other element removed: ");
    list_t* skip2=remove_nth_elements(l20,2);
    fprint_list(stdout,skip2);
    fprintf(stdout,"\n\n");

    fprint_list(stdout,l20);
    fprintf(stdout,"\nwith every third element removed: ");
    list_t* skip3=remove_nth_elements(l20,3);
    fprint_list(stdout,skip3);
    fprintf(stdout,"\n\n");


    list_t* lists[]={0,
                     list(1,1),
                     list(2,1,2),
                     list(3,1,2,3),
                     list(4,1,2,3,4),
                     list(5,1,2,3,4,5),
                     list(6,1,2,3,4,5,6),
                     l10,
                     l20};
    int nlists=sizeof(lists)/sizeof(lists[0]);
    int i;
    for(i=0;i<nlists;++i){
        list_t* l=lists[i];
        fprintf(stdout,"List ");
        fprint_list(stdout,l);
        fprintf(stdout,"\nwith the two half swapped: ");
        list_t* swapped=swap_halves(l);
        fprint_list(stdout,swapped);
        fprintf(stdout,"\n\n");
    }
            
    return 0;
}
