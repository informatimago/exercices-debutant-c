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
#include <assert.h>


////////////////////////////////////////////////////////////////////////////////
//
// Our simplistic error management "library".
//
////////////////////////////////////////////////////////////////////////////////

#define unused    __attribute__ ((unused))
#define noreturn  __attribute__ ((__noreturn__))


noreturn void error(int exitCode,const char* formatControl,...){
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


////////////////////////////////////////////////////////////////////////////////
//
// Première solution: nous implantons notre propre structure de donnée de liste chaînée.
//
////////////////////////////////////////////////////////////////////////////////


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


// un prédicat pour les nombre impairs:
int oddp(list_element_t element,unused void* data){
    return (element&1)==1;
}

// un prédicat pour les nombre pairs:
int evenp(list_element_t element,unused void* data){
    return (element&1)==0;
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
// seconde moitié.  Ainsi nous allons tester rest(tortue) et
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


// Deuxième groupe de trois fonctions:

list_t* repeat_list_elements(list_t* list,list_t* repeat_counts){
    list_t* result=0;
    while(list){
        int i=0;
        for(i=0;i<element(repeat_counts);++i){
            result=cons(element(list),result);
        }
        list=rest(list);
        repeat_counts=rest(repeat_counts);
    }
    return result;
}


// Pour comparer, voici une solution utilisant des vecteurs:

// Comme nous ne savons pas à l'avance combien d'élément le résultat aura,
// nous devons le calculer (c'est la somme des facteurs de répétition),
// et allouer le résultat dynamiquement (au moment de l'exécution, avec malloc).

int vector_sum(const int nelements,const int vector[]){
    int total=0;
    int i;
    for(i=0;i<nelements;++i){
        total+=vector[i];
    }
    return total;
}

int* repeat_vector_elements(const int nelements,const int vector[],const int repeat_counts[],int* total){
    int total_count=vector_sum(nelements,repeat_counts);
    int* result=check_not_null(malloc(sizeof(int)*total_count),
                               EX_OSERR,"Out of Memory");
    int dst=total_count;
    int v;
    for(v=0;v<nelements;++v){
        int element=vector[v];
        int count=repeat_counts[v];
        int i;
        for(i=0;i<count;++i){
            --dst;
            result[dst]=element;
        }
    }
    (*total)=total_count;
    return result;
}

#define countof(v) (sizeof(v)/sizeof((v)[0]))

void fprint_vector(FILE* out,const int nelements,const int vector[]){
    if(nelements==0){
        fprintf(out,"[]");
    }else{
        const char* sep="[";
        int i;
        for(i=0;i<nelements;++i){
            fprintf(out,"%s%d",sep,vector[i]);
            sep=" ";
        }
        fprintf(out,"]");
    }
}


// Pour trouver le dernier élément, on commence par trouver le dernier noeud.
// (En plus, le dernier noeud nous sera utile pour écrire nconc).

list_t* last(list_t* list){
    if(rest(list)){
        return last(rest(list));
    }else{
        return list;
    }
}

int last_element(list_t* list){
    return element(last(list));
}


// Pour odd_first, nous allons extraire les impairs et les pairs dans
// deux listes séparées, puis nous allons les concaténer ensemble.
// Comme les listes à concaténer seront des listes fraichement
// allouée, nous allons directement modifier la première pour y
// attacher la seconde, avec nconc, au lieu d'utiliser append qui
// copierait la première liste.

list_t* nconc(list_t* list1,list_t* list2){
    list_t* last_node=last(list1);
    set_rest(last_node,list2);
    return list1;
}

list_t* odd_first(list_t* list){
    return nconc(filter(list,evenp,0),filter(list,oddp,0));
}



// Et maintenant, des petits tests:

void tester_ma_structure_de_liste(){

    printf("\nTrois premier exercices sur les listes\n\n");

    list_t* l10=list(10,0,1,2,3,4,5,6,7,8,9);
    {
        fprint_list(stdout,l10);
        fprintf(stdout,"\nwith odd number removed: ");
        list_t* even=remove_odd_elements(l10);
        fprint_list(stdout,even);
        fprintf(stdout,"\n\n");
    }

    list_t* l20=list(20,
                     0,1,2,3,4,5,6,7,8,9,
                     10,11,12,13,14,15,16,17,18,19);
    {
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
    }

    {
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
    }

    printf("\nTrois exercices suivants sur les listes\n\n");

    {
        int nelements=5;
        list_t* hundreds=list(nelements,100,200,300,400,500);
        list_t* counts=list(nelements,4,2,0,3,1);
        list_t* repeated=repeat_list_elements(hundreds,counts);
        fprintf(stdout,"(repeat_list_elements   ");
        fprint_list(stdout,hundreds);
        fprintf(stdout," ");
        fprint_list(stdout,counts);
        fprintf(stdout,") --> ");
        fprint_list(stdout,repeated);
        fprintf(stdout,"\n");
    }

    {
        int hundreds[]={100,200,300,400,500};
        int counts[]={4,2,0,3,1};
        int nelements=countof(hundreds);
        assert(nelements==countof(counts));
        int total=0;
        int* repeated=repeat_vector_elements(nelements,hundreds,counts,&total);
        fprintf(stdout,"(repeat_vector_elements ");
        fprint_vector(stdout,nelements,hundreds);
        fprintf(stdout," ");
        fprint_vector(stdout,nelements,counts);
        fprintf(stdout,") --> ");
        fprint_vector(stdout,total,repeated);
        fprintf(stdout,"\n");
    }


    {
        list_t* small=list(9,1,2,3,4,5,6,7,8,9);
        list_t* oflist=odd_first(small);
        fprintf(stdout,"(odd_first ");
        fprint_list(stdout,small);
        fprintf(stdout,") --> ");
        fprint_list(stdout,oflist);
        fprintf(stdout,"\n");
    }

    {
        list_t* small=list(9,1,2,3,4,5,6,7,8,9);
        int le=last_element(small);
        fprintf(stdout,"(last_element ");
        fprint_list(stdout,small);
        fprintf(stdout,") --> %d\n",le);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Deuxième solution: il y a dans /usr/include/sys un fichier queue.h
// qui défini des macros C implantant plusieurs variantes de structures de liste
// chaînée.  Nous allons l'utiliser ici.  man 3 queue
//
////////////////////////////////////////////////////////////////////////////////

/*

Noter  la différence  entre la  façon  de définir  les liste  chaînées
proposée plus haut (à la mode de lisp), et celle implantée dans queue.h:


Fondamentallement la façon lisp de faire des listes, est de définir
une cellule paire (cons cell) contenant deux références sur des objets
quelconques (deux pointeurs).  On appelle conventionellement (et
historiquement) ces deux références le car et le cdr.  (Gauche,
droite), (first, rest), peut importe le nom, le tout est d'identifier
chacun des deux champs de cette structure.   Ces paires sont trés
générales et permettent d'implanter toutes sortes de structures de
données.  Pour implanter une structure de liste chaînée, on fait
pointer chaque cdr sur l'élément suivant.  Dans ce cas, on appelle le
car first, et le cdr rest, puisque (first list) = (car list) va donner le premier
élement de la liste, et (rest list) = (cdr list) va donner le reste de
la liste.  Dans le car, on stockera une référence sur l'élément. En
général ce sera un pointeur vers un autre objet (et même peut être sur
une autre liste, une sous-liste).


     (1 2 3 43189031289381209381209389013812839183091)

     +---+---+   +---+---+   +---+---+   +---+---+
     | * | * |-->| * | * |-->| * | * |-->| * |NIL|
     +---+---+   +---+---+   +---+---+   +---+---+
       |           |           |           |
       v           v           v           v
     +---+       +---+       +---+       +-------------------------------------------+
     | 1 |       | 2 |       | 3 |       | 43189031289381209381209389013812839183091 |
     +---+       +---+       +---+       +-------------------------------------------+


Mais quand cet élément est un petit entier (un fixnum, un entier de
petite taille fixe), on pourra l'enregistrer directement à la place
d'un pointeur, dans le champ CAR.


     +---+---+   +---+---+   +---+---+   +---+---+
     | 1 | * |-->| 2 | * |-->| 3 | * |-->| * |NIL|
     +---+---+   +---+---+   +---+---+   +---+---+
                                           |
                                           v
                                         +-------------------------------------------+
                                         | 43189031289381209381209389013812839183091 |
                                         +-------------------------------------------+


Ceci est possible en lisp, parce que le type des objets peut être
identifié au pendant l'exécution, ainsi on peut facilement stocker
dans une variable ou un champ d'une structure, des pointeurs sur des
objets de types différents.



Mais en C, en général on n'identifie pas le type d'une valeur à
l'exécution. Le type est une notion attachée aux variables, et non pas
aux valeurs en C.  Ainsi, la même séquence de bit en mémoire va être
interpréter de manière différente en fonction du type déclaré de la
variable qui donne accès à cette séquence de bit en mémoire.

On peut définir des structures de données en C qui implante un système
de typage des valeurs similaire à lisp, mais c'est plus de
travail. (On le fait bien sur quand on doit implanter un système lisp
en C).


Donc, en C, il va être plus malaisé de définir une struture cellule
permettant de faire référence à des objets de n'importe quel type, car
on doit déclarer le type des champs dans la cellule, et c'est ces
types déclarés qui imposeront une interprétation des valeurs.


Si on fait une liste d'entier, puis une liste de flottants, on devra
déclarer deux structures:

    struct liste_entier {
        int element;
        struct liste_entier* suivant;
    };

    struct liste_flottant {
        float element;
        struct liste_flottant* suivant;
    };

et ceci alors même que ces structures auront exactement la même
représentation en mémoire et que toutes les fonctions que l'on
définira pour l'une, seront identiques aux fonctions définies pour
l'autre (au type de l'élément près).


De surcroît,  on défini souvent la  liste en insérant le  pointeur sur
l'élément suivant  directement dans  une structure  d'objet existante.
Par exemple,  si on a  une structure  personne, et on  veut l'intégrer
dans une  liste de  frères et  soeurs, on ajoute  un pointeur vers le
frère suivant.  Et si on veut l'intégrer dans une liste d'employés, on
ajoutera un pointeur vers l'employé suivant:

    struct personne {
        char* nom;
        char* prenom;
        char  sexe;
        struct personne* petit_frere_ou_soeur_suivant;
        struct personne* employe_suivant;
    };

Cette façon de procéder pose plein de problèmes: que se passe-t'il
quand la personne est employée de plusieurs entreprises (simultanément
ou consécutivement)?  Que se passe-t'ils avec les demi-frêres et les
demi-soeurs? (Les frâtries peuvent s'intersecter, mais ne se
reconvrent pas forcément).  En général, le problème, c'est que les
listes et les collections où l'on rassemble des objets souvent ne sont
pas des propriétés intrinsèques de ces objets, mais des constructions
externes.  Un objet peut avoir des listes, mais pas en tant qu'élément
mais en tant que propriétaire: un personne peut avoir une liste
d'enfants (et il n'en fait pas partie); une entreprise peut avoir une
liste d'employées.  Et une personne peut faire partie de plusieurs liste
d'enfants (en tant enfant de son père biologique, sa mère biologique,
son père adoptif, sa mère adoptif, de la Patrie), et de plusieurs
liste d'employés, etc.

Ceci dit, si on peut trouver une situation où on a vraiement qu'une
seule liste dont l'objet peut faire partie, (par exemple, dans le cas
où on ne garde pas des références unique sur des structures, mais où
on a des entiers dont on fait une copie à chaque fois qu'on l'inclue
dans une nouvelle liste) on peut considérer fusionner la structure de
donnée et les pointeurs vers l'élément suivant de la liste dans une
seule et même structure.  Et on peut même avoir plusieurs listes comme
les frères et les employés ci-dessus.

C'est une situation qui est facilement implantable en C, est c'est
pourquoi il est fourni ce fichier sys/queue.h qui défini des macros
pour générer le code identique qui devra être générer pour gèrer
chaque liste.

Noter également qu'on va utiliser en C une structure tête de liste
pour chaque type de liste; ceci est en général inutile en lisp, où on
va référencer directement la première cellule pair.  Plus
fondamentalement, la tête de liste qui doit être modifiée lorsqu'on
insère un élement "en tête de liste", va signifier un style de
programmation plus procédural, usant des mutations (modifications de
variables) alors qu'en lisp on va pouvoir adopter un style plus
fonctionnel, où l'insertion en tête de liste se ramène simplement à la
création d'une nouvelle cellule ayant comme reste la liste précédente,
c'est à dire l'ancien premier élement de la liste, qui devient ainsi
maintenant le deuxième élement.


    façon C:

      +--------------+           +----------------------------+
      | famille: Di  |           | entreprise:                |
      +--------------+           | Négoce de Vin Aibu & fils  |
      | frères       |--+        +----------------------------+
      +--------------+  |     +--| employés                   |
                        |     |  +----------------------------+
   +----------------+   |     |
   | tête de liste  |<--|-----+
   +----------------+   |
   |  premier       |   |   +----------------+
   +----------------+   +-->| tête de liste  |
            |               +----------------+
            |               |  premier       |
            |               +----------------+
            |                   |
            |                   |
            v                   v
        +-------+           +-------+           +-------+
        | Jean  |           | Alain |           | Amar  |
        +-------+           +-------+           +-------+
        | Aibu  |           | Di    |           | Di    |
        +---- --+           +-------+           +-------+
        | M     |           | M     |           | M     |
        +-------+           +-------+           +-------+
        | fsuiv |           | fsuiv |---------->| fsuiv |
        +-------+           +-------+           +-------+
        | esuiv |---------->| esuiv |           | esuiv |
        +-------+           +-------+           +-------+


    façon lisp:

      +--------------+           +----------------------------+
      | famille: Di  |           | entreprise:                |
      +--------------+           | Négoce de Vin Aibu & fils  |
      | frères       |--+        +----------------------------+
      +--------------+  |     +--| employés                   |
                        |     |  +----------------------------+
      +-----------------|-----+
      |                 |
      |                 |              +---+---+    +---+---+
      |                 +------------->| * | * |--->| * |NIL|
      |                                +---+---+    +---+---+
      |                                  |            |
      |                                  |            |
      |                                  |            |
      |   +---+---+          +---+---+   |            |
      +-->| * | * |--------->| * |NIL|   |            |
          +---+---+          +---+---+   |            |
            |                 |          |            |
            |                 |   +------+            |
            |                 |   |                   |
            v                 v   v                   v
        +-------+           +-------+           +-------+
        | Jean  |           | Alain |           | Amar  |
        +-------+           +-------+           +-------+
        | Aibu  |           | Di    |           | Di    |
        +-------+           +-------+           +-------+
        | M     |           | M     |           | M     |
        +-------+           +-------+           +-------+


*/


#include <sys/queue.h>

SLIST_HEAD(entete_liste_de_int,noeud_de_liste_de_int);
struct noeud_de_liste_de_int {
    int element;
    SLIST_ENTRY(noeud_de_liste_de_int) int_suivant;
};

// creation d'un nouveau noeud:
struct noeud_de_liste_de_int* nouveau_noeud_de_liste_de_int(int element){
    struct noeud_de_liste_de_int* noeud=check_not_null(malloc(sizeof(*noeud)),EX_OSERR,"Out of Memory");
    noeud->element=element;
    return noeud;
}

// creation d'une nouvelle liste:
struct entete_liste_de_int* nouvelle_liste_de_int(int count,int element,...){
    struct entete_liste_de_int* liste=check_not_null(malloc(sizeof(*liste)),EX_OSERR,"Out of Memory");
    // We cannot write (*liste)=SLIST_HEAD_INITIALIZER(*liste);
    // so we have to initialize a list header variable, and then copy its data to the
    // dynamically allocated list header.
    struct entete_liste_de_int  liste_initialisee=SLIST_HEAD_INITIALIZER(liste_initialisee);
    memcpy(liste,&liste_initialisee,sizeof(liste_initialisee));
    SLIST_INIT(liste);    // initialization de la liste
    
    // Next we allocate the element nodes.
    va_list arguments;
    switch(count){
      case 0:
          return liste;
      case 1:{
          struct noeud_de_liste_de_int* noeud=nouveau_noeud_de_liste_de_int(element);
          SLIST_INSERT_HEAD(liste,noeud,int_suivant);
          return liste;
      }
      default:{
          // Remarquez comment nous devons maintenant distinguer
          // l'insertion du premier élément, de l'insertion des
          // élements suivant (SLIST_INSERT_HEAD
          // vs. SLIST_INSERT_AFTER); et comment nous devons indiquer
          // int_suivant, le champ utilisé par la structure de liste.
          // Ceci sera une catastrophe pour la maintenance de ce
          // programme: quand le nom du champ changera, il faudra
          // modifier toutes ces occurences.
          struct noeud_de_liste_de_int* dernier=nouveau_noeud_de_liste_de_int(element);
          SLIST_INSERT_HEAD(liste,dernier,int_suivant);
          va_start(arguments,element);
          while(--count>0){
              list_element_t element=va_arg(arguments,list_element_t);
              struct noeud_de_liste_de_int* noeud=nouveau_noeud_de_liste_de_int(element);
              SLIST_INSERT_AFTER(dernier,noeud,int_suivant);
              dernier=noeud;
          }
          va_end(arguments);
          return liste;
      }
    }
}    


void fprint_liste_de_int(FILE* out,struct entete_liste_de_int* liste){
    if(!SLIST_EMPTY(liste)){
        struct noeud_de_liste_de_int* noeud;
        const char* sep="(";
        SLIST_FOREACH(noeud,liste,int_suivant){
            fprintf(out,"%s%d",sep,noeud->element);
            sep=" ";
        }
        fprintf(out,")");
    }else{
        fprintf(out,"()");
    }
}



struct entete_liste_de_int* filtre_de_liste_de_int(struct entete_liste_de_int* liste,
                                                   predicate_f predicate,void* data){
    struct entete_liste_de_int* resultat=nouvelle_liste_de_int(0,0);
    if(!SLIST_EMPTY(liste)){
        struct noeud_de_liste_de_int* dernier=0;
        struct noeud_de_liste_de_int* noeud=0;
        SLIST_FOREACH(noeud,liste,int_suivant){
            if(!predicate(noeud->element,data)){
                // Notez comment on doit copier ici toute la structure
                // (heureusement on n'a qu'un int, mais ce serait
                // catastrophique si on avait une plus grosse
                // structure):
                struct noeud_de_liste_de_int* nouveau=nouveau_noeud_de_liste_de_int(noeud->element);
                if(dernier){
                    SLIST_INSERT_AFTER(dernier,nouveau,int_suivant);
                }else{
                    SLIST_INSERT_HEAD(resultat,nouveau,int_suivant);
                }
                dernier=nouveau;
            }
        }
    }
    return resultat;
}

// un filtre pour enlever les éléments impairs:
struct entete_liste_de_int* remove_odd_elements_de_liste_de_int(struct entete_liste_de_int* liste){
    return filtre_de_liste_de_int(liste,oddp,0);
}

// un filtre pour enlever les n-ièmes élements.
// Note: on fait une copie au lieu de couper la liste originale.
struct entete_liste_de_int* remove_nth_elements_de_liste_de_int(struct entete_liste_de_int* liste,int n){
    closure_every_nth_t closure={0,n};
    return filtre_de_liste_de_int(liste,every_nth,&closure);
}


void tester_structure_de_liste_avec_queue(){

    printf("\nTrois premier exercices sur les listes IMPLANTÉS AVEC sys/queue.h\n\n");

    struct entete_liste_de_int* l10=nouvelle_liste_de_int(10,0,1,2,3,4,5,6,7,8,9);
    {
        fprint_liste_de_int(stdout,l10);
        fprintf(stdout,"\nwith odd number removed: ");
        struct entete_liste_de_int* even=remove_odd_elements_de_liste_de_int(l10);
        fprint_liste_de_int(stdout,even);
        fprintf(stdout,"\n\n");
    }

    struct entete_liste_de_int* l20=nouvelle_liste_de_int(20,
                     0,1,2,3,4,5,6,7,8,9,
                     10,11,12,13,14,15,16,17,18,19);
    {
        fprint_liste_de_int(stdout,l20);
        fprintf(stdout,"\nwith every other element removed: ");
        struct entete_liste_de_int* skip2=remove_nth_elements_de_liste_de_int(l20,2);
        fprint_liste_de_int(stdout,skip2);
        fprintf(stdout,"\n\n");

        fprint_liste_de_int(stdout,l20);
        fprintf(stdout,"\nwith every third element removed: ");
        struct entete_liste_de_int* skip3=remove_nth_elements_de_liste_de_int(l20,3);
        fprint_liste_de_int(stdout,skip3);
        fprintf(stdout,"\n\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Le programme principal: testons les deux solutions.
//
////////////////////////////////////////////////////////////////////////////////

int main(){
    tester_ma_structure_de_liste();
    tester_structure_de_liste_avec_queue();
    return 0;
}
