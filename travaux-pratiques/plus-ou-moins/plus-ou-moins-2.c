/*

Un petit jeu que j'appelle « Plus ou moins ».

Le principe est le suivant.

    L'ordinateur tire au sort un nombre entre 1 et 100.

    Il vous demande de deviner le nombre. Vous entrez donc un nombre entre 1 et 100.

    L'ordinateur compare le nombre que vous avez entré avec le nombre
    « mystère » qu'il a tiré au sort. Il vous dit si le nombre mystère
    est supérieur ou inférieur à celui que vous avez entré.

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

#define unused   __attribute__ ((unused))
#define noreturn __attribute__ ((__noreturn__))

noreturn
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


////////////////////////////////////////////////////////////////////////
//
// Nombres aléatoires
//
////////////////////////////////////////////////////////////////////////

void initialiser(){
    srandom(clock());
}

int choisir_un_nombre_aleatoire_dans_l_intervale(int min,int max){
    return random()%(max-min+1)+min;
}

////////////////////////////////////////////////////////////////////////
//
// Jeu du plus-ou-moins.
//
// On defini le jeu de manière abstraite, en implémentant les règles
// du jeu, indépendement de l'interface utilisateur.
//
////////////////////////////////////////////////////////////////////////

typedef enum {moins=0,egal,plus,abandon} plus_ou_moins_result;

typedef struct joueur_plus_ou_moins{
    void* data;
    void (*debut)(struct joueur_plus_ou_moins* joueur,int min,int max);
    // Appelée au début du jeu pour informer le joueur des limites.
    int (*jouer)(struct joueur_plus_ou_moins* joueur);
    // Appelée pour permettre au joueur de faire son choix.
    // Note: un résutat négatif indique l'abandon.
    void (*resultat)(struct joueur_plus_ou_moins* joueur,int choix,plus_ou_moins_result resultat);
    // Appelée pour indiquer au joueur le résultat de son choix.
    void (*fin)(struct joueur_plus_ou_moins* joueur,plus_ou_moins_result resultat_final);
    // Appelée pour signaler la fin du jeu au joueur.
} joueur_plus_ou_moins_t;

typedef struct {
    int min;
    int max;
    joueur_plus_ou_moins_t* joueur;
    int nombre_mystere;
} plus_ou_moins_t;


plus_ou_moins_t* nouveau_jeu_plus_ou_moins(int min,int max,joueur_plus_ou_moins_t* joueur){
    assert(min<max);
    plus_ou_moins_t* jeu=check_not_null(malloc(sizeof(*jeu)),EX_OSERR,"Out of Memory");
    jeu->min=min;
    jeu->max=max;
    jeu->joueur=joueur;
    jeu->nombre_mystere=choisir_un_nombre_aleatoire_dans_l_intervale(min,max);
    return jeu;
}

plus_ou_moins_result plus_ou_moins_essayer_nombre(plus_ou_moins_t* jeu,int essai){
    if(essai<0){
        return abandon;
    }else if(jeu->nombre_mystere<essai){
        return moins;
    }else if(jeu->nombre_mystere>essai){
        return plus;
    }else{
        return egal;
    }
}

void plus_ou_moins_jouer(plus_ou_moins_t* jeu){
    joueur_plus_ou_moins_t* joueur=jeu->joueur;
    joueur->debut(joueur,jeu->min,jeu->max);
    plus_ou_moins_result resultat=plus;
    do{
        int choix=joueur->jouer(joueur);
        resultat=plus_ou_moins_essayer_nombre(jeu,choix);
        joueur->resultat(joueur,choix,resultat);
    }while((resultat!=egal)&&(resultat!=abandon));
    joueur->fin(joueur,resultat);
}



////////////////////////////////////////////////////////////////////////
//
// En ayant ainsi un jeu abstrait, on peut l'utiliser avec divers
// joueurs, lesquels peuvent avoir différent interfaces utilisateurs.
//    
// Ici on représente un joueur humain utilisant un interface utilisateur CLI:
//
////////////////////////////////////////////////////////////////////////

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

typedef struct {
    int min;
    int max;
    char* invite;
}  cli_data_t;


void cli_debut(joueur_plus_ou_moins_t* joueur,int min,int max){
    // Appelée au début du jeu pour informer le joueur des limites.
    cli_data_t* data=check_not_null(malloc(sizeof(*data)),EX_OSERR,"Out of Memory");
    data->min=min;
    data->max=max;
    int size=1+snprintf(0,0,"Devinez un entier entre %d et %d",data->min,data->max);
    data->invite=check_not_null(malloc(size),EX_OSERR,"Out of Memory");
    snprintf(data->invite,size,"Devinez un entier entre %d et %d",data->min,data->max);
    joueur->data=data;
}

int cli_jouer(joueur_plus_ou_moins_t* joueur){
    // Appelée pour permettre au joueur de faire son choix.
    // Note: un résutat négatif indique l'abandon.
    cli_data_t* data=joueur->data;
    return(demander_un_nombre(data->invite));
}

void cli_resultat(unused joueur_plus_ou_moins_t* joueur,unused int choix,plus_ou_moins_result resultat){
    // Appelée pour indiquer au joueur le résultat de son choix.
    static const char* label[]={"moins","egal","plus"};
    indiquer_ordre(label[resultat]);
}

void cli_fin(unused joueur_plus_ou_moins_t* joueur,plus_ou_moins_result resultat_final){
    // Appelée pour signaler la fin du jeu au joueur.
    if(resultat_final==egal){
        printf("Bravo, vous avez trouve le nombre mystere !!!\n");
    }else{
        printf("Lacheur !\n");
    }
}


joueur_plus_ou_moins_t* nouveau_cli_joueur(){
    joueur_plus_ou_moins_t* joueur=check_not_null(malloc(sizeof(*joueur)),EX_OSERR,"Out of Memory");
    joueur->debut    = &cli_debut;
    joueur->jouer    = &cli_jouer;
    joueur->resultat = &cli_resultat;
    joueur->fin      = &cli_fin;
    return joueur;
}


////////////////////////////////////////////////////////////////////////
//
// Ici on représente un programme joueur, jouant tout seul (il affiche
// des messages sur stdout pour qu'on suive ce qu'il fait):
//
////////////////////////////////////////////////////////////////////////

typedef struct {
    int min;
    int max;    
} auto_data_t;

void auto_debut(joueur_plus_ou_moins_t* joueur,int min,int max){
    // Appelée au début du jeu pour informer le joueur des limites.
    auto_data_t* data=check_not_null(malloc(sizeof(*data)),EX_OSERR,"Out of Memory");
    data->min=min;
    data->max=max;
    joueur->data=data;
    printf("jeu:    Devinez un entier entre %d et %d\n",data->min,data->max);
}

int auto_jouer(joueur_plus_ou_moins_t* joueur){
    // Appelée pour permettre au joueur de faire son choix.
    // Note: un résutat négatif indique l'abandon.
    auto_data_t* data=joueur->data;
    assert(data->min!=data->max);
    int essai=(data->min+data->max+1)/2;
    printf("joueur: Est-ce %d ?\n",essai);
    return essai;
}

void auto_resultat(unused joueur_plus_ou_moins_t* joueur,unused int choix,plus_ou_moins_result resultat){
    // Appelée pour indiquer au joueur le résultat de son choix.
    auto_data_t* data=joueur->data;
    static const char* label[]={"moins","egal","plus"};
    printf("jeu:    "); indiquer_ordre(label[resultat]);
    switch(resultat){
      case moins:
          data->max=choix;
          break;
      case plus:
          data->min=choix;
          break;
      case egal:
          data->min=choix;
          data->max=choix;
          printf("joueur: J'ai trouvé %d !\n",choix);
          break;
      default:
          assert(0); // should not occur.
          break;
    }
}

void auto_fin(unused joueur_plus_ou_moins_t* joueur,plus_ou_moins_result resultat_final){
    // Appelée pour signaler la fin du jeu au joueur.
    if(resultat_final==egal){
        printf("jeu:    Bravo, vous avez trouve le nombre mystere !!!\n");
    }else{
        printf("jeu:    Lacheur !\n");
    }
}


joueur_plus_ou_moins_t* nouveau_auto_joueur(){
    joueur_plus_ou_moins_t* joueur=check_not_null(malloc(sizeof(*joueur)),EX_OSERR,"Out of Memory");
    joueur->debut    = &auto_debut;
    joueur->jouer    = &auto_jouer;
    joueur->resultat = &auto_resultat;
    joueur->fin      = &auto_fin;
    return joueur;
}

////////////////////////////////////////////////////////////////////////
//
// Testons les deux joeurs:
//
////////////////////////////////////////////////////////////////////////

void test_plus_ou_moins(){
    printf("\n\nJoueur automatique\n\n");
    plus_ou_moins_jouer(nouveau_jeu_plus_ou_moins(1,100,nouveau_auto_joueur()));
    printf("\n\nJoueur humain (c'est a vous!)\n\n");
    plus_ou_moins_jouer(nouveau_jeu_plus_ou_moins(1,100,nouveau_cli_joueur()));
}


////////////////////////////////////////////////////////////////////////
// 
// Lorsque l'utilisateur a trouvé le nombre mystère, le programme
// s'arrête. Pourquoi ne pas demander s'il veut faire une autre partie ?
// 
// Si vous faites ça, il vous faudra faire une boucle qui englobera la
// quasi-totalité de votre programme. Cette boucle devra se répéter TANT
// QUE l'utilisateur n'a pas demandé à arrêter le programme. Je vous
// conseille de rajouter une variable booléenne continuerPartie
// initialisée à 1 au départ. Si l'utilisateur demande à arrêter le
// programme, vous mettrez la variable à 0 et le programme s'arrêtera.
// 
////////////////////////////////////////////////////////////////////////

typedef joueur_plus_ou_moins_t* (*nouveau_joueur_pr)();


int ask(const char* prompt,const char* no,const char* yes){
    while(1){
        printf("%s (%s/%s) ? ",prompt,no,yes);
        fflush(stdout);
        fflush(stdin);
        char buffer[81];
        clearerr(stdin);
        if(scanf("%80s",buffer)){
            if(strcmp(no,buffer)==0){
                return 0;
            }
            if(strcmp(yes,buffer)==0){
                return 1;
            }
            printf("Reponse invalide: %s\n",buffer);
        }else if(feof(stdin)){
            return -1;
        }else{
            fprintf(stderr,"Error while reading answer\n");
            return -2;
        }
    }
}

void plusieurs_plus_ou_moins(nouveau_joueur_pr joueur_createur){
    int encore=0;
    do{
        plus_ou_moins_jouer(nouveau_jeu_plus_ou_moins(1,100,joueur_createur()));
        encore=ask("Voulez vous essayer encore","non","oui");
    }while(encore>0);
}

int main(const int argc,const char* argv[]){
    initialiser();
    if((argc>1)&&(strcmp("test",argv[1])==0)){
        test_plus_ou_moins();
    }else{
        plusieurs_plus_ou_moins(&nouveau_cli_joueur);
    }
    return 0;
}
