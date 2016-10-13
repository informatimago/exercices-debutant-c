/*
 un petit jeu que j'appelle « Plus ou moins ».

Le principe est le suivant.

    L'ordinateur tire au sort un nombre entre 1 et 100.

    Il vous demande de deviner le nombre. Vous entrez donc un nombre entre 1 et 100.

    L'ordinateur compare le nombre que vous avez entré avec le nombre « mystère » qu'il a tiré au sort. Il vous dit si le nombre mystère est supérieur ou inférieur à celui que vous avez entré.

    Puis l'ordinateur vous redemande le nombre.

    … Et il vous indique si le nombre mystère est supérieur ou inférieur.

    Et ainsi de suite, jusqu'à ce que vous trouviez le nombre mystère.

Le but du jeu, bien sûr, est de trouver le nombre mystère en un minimum de coups.

*/

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



void initialiser(){
    srandom(clock());
}

int choisir_un_nombre_aleatoire_dans_l_intervale(int min,int max){
    return random()%(max-min+1)+min;
}

// On defini le jeu de manière abstraite, en implémentant les règles
// du jeu, indépendement de l'interface utilisateur.

typedef struct {
    int nombre_mystere;
} plus_ou_moins_t;


plus_ou_moins_t* nouveau_jeu_plus_ou_moins(int min,int max){
    plus_ou_moins_t* jeu=check_not_null(malloc(sizeof(*jeu)),EX_OSERR,"Out of Memory");
    jeu->nombre_mystere=choisir_un_nombre_aleatoire_dans_l_intervale(min,max);
    return jeu;
}

typedef enum {moins=0,egal,plus} plus_ou_moins_result;

plus_ou_moins_result plus_ou_moins_essayer_nombre(plus_ou_moins_t* jeu,int essai){
    if(jeu->nombre_mystere<essai){
        return moins;
    }else if(jeu->nombre_mystere>essai){
        return plus;
    }else{
        return egal;
    }
}

// En ayant ainsi un jeu abstrait, on peut l'utiliser dans divers
// interface utilisateurs, ou on peut l'intégrer avec d'autre joueurs.
    
// Ici on a un interface utilisateur CLI:

int demander_un_nombre(const char* invite){
    printf("%s : ",invite);
    fflush(stdout); // tres important!
    int nombre=0;
    scanf("%i",&nombre);
    return nombre;
}

void indiquer_ordre(const char* ordre){
    printf("C'est %s !\n",ordre);
}

void plus_ou_moins(){
    static const char* label[]={"moins","egal","plus"};
    plus_ou_moins_t* jeu=nouveau_jeu_plus_ou_moins(1,100);
    plus_ou_moins_result resultat=moins;
    while(resultat!=egal){
        int nombre_teste=demander_un_nombre("Devinez un entier entre 1 et 100");
        resultat=plus_ou_moins_essayer_nombre(jeu,nombre_teste);
        indiquer_ordre(label[resultat]);
    }
    printf("Bravo, vous avez trouve le nombre mystere !!!\n");
}


// Ici on a un programme jouant tout seul:

typedef struct {
    int min;
    int max;
    plus_ou_moins_t* jeu;
} joueur_t;

joueur_t* nouveau_joueur(int min,int max){
    assert(min<max);
    joueur_t* joueur=check_not_null(malloc(sizeof(*joueur)),EX_OSERR,"Out of Memory");
    joueur->min=min;
    joueur->max=max;
    joueur->jeu=nouveau_jeu_plus_ou_moins(min,max);
    return joueur;
}

int jouer(joueur_t* joueur){
    if(joueur->min==joueur->max){
        printf("joueur: J'ai trouvé %d\n",joueur->min);
        return 0; //fini
    }
    int essai=(joueur->min+joueur->max+1)/2;
    printf("joueur: Est-ce %d ?\n",essai);
    plus_ou_moins_result result=plus_ou_moins_essayer_nombre(joueur->jeu,essai);
    static const char* label[]={"moins","egal","plus"};
    printf("jeu:    "); indiquer_ordre(label[result]);
    switch(result){
      case moins:
          joueur->max=essai;
          break;
      case plus:
          joueur->min=essai;
          break;
      case egal:
          joueur->min=essai;
          joueur->max=essai;
          break;
    }
    return 1; // jouer encore!
}

void plus_ou_moins_auto(){
    joueur_t* joueur=nouveau_joueur(1,100);
    while(jouer(joueur));
    printf("jeu:    "); printf("Bravo, vous avez trouve le nombre mystere !!!\n");
}    

int main(){
    initialiser();
    plus_ou_moins_auto();
    plus_ou_moins();
    return 0;
}
