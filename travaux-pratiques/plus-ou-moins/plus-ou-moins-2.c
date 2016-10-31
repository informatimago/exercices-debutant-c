/*

Un petit jeu que j'appelle « Plus ou moins ».

Le principe est le suivant.

    L'ordinateur tire au sort un nombre entre 1 et 100.

    Il vous demande de deviner le nombre. Vous entrez donc un nombre
    entre 1 et 100.

    L'ordinateur compare le nombre que vous avez entré avec le nombre
    « mystère » qu'il a tiré au sort. Il vous dit si le nombre mystère
    est supérieur ou inférieur à celui que vous avez entré.

    Puis l'ordinateur vous redemande le nombre.

    … Et il vous indique si le nombre mystère est supérieur ou inférieur.

    Et ainsi de suite, jusqu'à ce que vous trouviez le nombre mystère.

Le but du jeu, bien sûr, est de trouver le nombre mystère en un
minimum de coups.


*   Faites un compteur de « coups ». Ce compteur devra être une
    variable que vous incrémenterez à chaque fois que vous passez dans
    la boucle. Lorsque l'utilisateur a trouvé le nombre mystère, vous
    lui direz « Bravo, vous avez trouvé le nombre mystère en 8 coups »
    par exemple.

*   Lorsque l'utilisateur a trouvé le nombre mystère, le programme
    s'arrête. Pourquoi ne pas demander s'il veut faire une autre
    partie ?

    Si vous faites ça, il vous faudra faire une boucle qui englobera
    la quasi-totalité de votre programme. Cette boucle devra se
    répéter TANT QUE l'utilisateur n'a pas demandé à arrêter le
    programme. Je vous conseille de rajouter une variable booléenne
    continuerPartie initialisée à 1 au départ. Si l'utilisateur
    demande à arrêter le programme, vous mettrez la variable à 0 et le
    programme s'arrêtera.

*   Implémentez un mode 2 joueurs ! Attention, je veux qu'on ait le
    choix entre un mode 1 joueur et un mode 2 joueurs !

    Vous devrez donc faire un menu au début de votre programme qui
    demande à l'utilisateur le mode de jeu qui l'intéresse.

    La seule chose qui changera entre les deux modes de jeu, c'est la
    génération du nombre mystère. Dans un cas ce sera un rand() comme
    on a vu, dans l'autre cas ça sera… un scanf.

*   Créez plusieurs niveaux de difficulté. Au début, faites un menu
    qui demande le niveau de difficulté. Par exemple :

        1 = entre 1 et 100 ;

        2 = entre 1 et 1000 ;

        3 = entre 1 et 10000.

    Si vous faites ça, vous devrez changer votre constante MAX… Eh
    oui, ça ne peut plus être une constante si la valeur doit changer
    au cours du programme ! Renommez donc cette variable en
    nombreMaximum (vous prendrez soin d'enlever le mot-clé const sinon
    ça sera toujours une constante !). La valeur de cette variable
    dépendra du niveau qu'on aura choisi.

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
#include <libgen.h>

// This unused attribute prevents warnings about unused variables or
// parameters.
#define unused   __attribute__ ((unused))

// This noreturn attributes indicates to the compiler that a function
// will never return (because it always calls exit() or abort()).
#define noreturn __attribute__ ((__noreturn__))

// This countof(a) macro returns the number of entries in the array a.
#define countof(a) (sizeof(a)/sizeof(a[0]))


// The standard assert() macro doesn't print the expression that is false.
// So here is my own assert, which stringify (transforms into a string)
// the expression, to print it when it's false.
#define stringify(thing) #thing
#define my_assert(expression) \
    if(!(expression)){ \
        fprintf(stderr,"%s:%d: in function %s: Assertion %s failed.\n",\
                __FILE__,__LINE__,__FUNCTION__,stringify(expression)); \
        abort(); }

// should_not_occur() is an assert that is always false, since it
// should not occur.
const int SHOULD_NOT_OCCUR=(0);
#define should_not_occur() my_assert(SHOULD_NOT_OCCUR)

// Prints an error, and exit with the exitCode.
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

// Returns the given address, unless it's null, in which case it calls
// error() with the remaining arguments.
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

// If result is not null, then it call error() with the remaining arguments.
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

// Returns a newly allocated mutable copy of the string.
// Once a string is declared with const char*,
// it cannot be passed anymore to functions expecting a char*,
// so we use this function to make a mutable copy.
char* string_copy(const char* string){
    char* copy=check_not_null(malloc(1+strlen(string)),EX_OSERR,"Out of Memory");
    strcpy(copy,string);
    return copy;
}


// Returns the exponential of the base to the power, for int arguments.
int expi(int base,int power){
    my_assert(power>=0);
    int exp=1;
    while(power!=0){
        if(power&1){
            exp*=base;
            --power;
        }else{
            exp*=exp;
            power/=2;
        }
    }
    return exp;
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


// Using scanf to read numbers when the input is not a number doesn't
// eat any input.  This leads to infinite loops.  So instead of using
// bare scanf on stdin, we read it line-by-line with fgets, and use
// sscanf to read words and numbers.  Thus next time, we get a new
// line, and new input.

char* read_word(FILE* input){
    static char line[81];
    static char word[81];
    if(fgets(line,sizeof(line),input)){
        int n=sscanf(line,"%80s",word);
        while((line[strlen(line)-1]!='\n')
              &&fgets(line,sizeof(line),input));
        if(n==0){
            return 0;
        }else{
            return word;
        }
    }
    return 0;
}

int demander_un_nombre(const char* invite){
    printf("%s : ",invite);
    fflush(stdout); // tres important!
    fflush(stdin); // important, in case of invalid input.
    int nombre=0;
    char* word=read_word(stdin);
    sscanf(word,"%20i",&nombre);
    return nombre;
}

void clear_screen(){
    // TODO: use termcap/terminfo to determine if we can better send a control sequence to clear the screen.
    char* slines=getenv("LINES");
    int lines=80;
    if(slines!=0){
        sscanf(slines,"%20i",&lines);
    }
    while(0<=--lines){
        printf("\n");
    }
    fflush(stdout);
}



typedef struct {
    int choix;
    const char* labels[8];
} option_t;

option_t options_booleennes[]={ {0,{"non","no","n",0}},
                                {1,{"oui","yes","o","y",0}},
                                {0,{0}}};


int demander(const char* prompt,const option_t options[]){
    while(1){
        printf("%s(",prompt);
        const char* sep="";
        int i;
        for(i=0;options[i].labels[0]!=0;++i){
            int j;
            for(j=0;options[i].labels[j]!=0;++j){
                printf("%s%s",sep,options[i].labels[j]);
                sep="/";
            }
        }
        printf(") ? ");
        fflush(stdout);
        fflush(stdin);
        clearerr(stdin);
        char* word=read_word(stdin);
        if(word){
           int i;
            for(i=0;options[i].labels[0]!=0;++i){
                int j;
                for(j=0;options[i].labels[j]!=0;++j){
                    if(strcmp(word,options[i].labels[j])==0){
                        return options[i].choix;
                    }
                }
            }
            printf("Reponse invalide: %s\n",word);
        }else if(feof(stdin)){
            return -1;
        }else{
            fprintf(stderr,"Error while reading answer\n");
            return -2;
        }
    }
}


////////////////////////////////////////////////////////////////////////
//
// Jeu du plus-ou-moins.
//
// On defini le jeu de manière abstraite, en implémentant les règles
// du jeu, indépendement de l'interface utilisateur.
//
////////////////////////////////////////////////////////////////////////
//
// On considère le jeu entre deux joueurs: un maitre de jeu, et un joueur.
// Le maitre de jeu choisi le nombre aléatoire, et indique au joueur la
// comparaison avec ses choix.  Si le maitre se trompe, il perd.
// Si le joueur trouve le nombre, il gagne.
//

typedef enum {moins=0,
              egal,
              plus,
              abandon_joueur,
              erreur_maitre,
              abandon_maitre,
} resultat_plus_ou_moins;

option_t options_comparaison[]={ {moins,{"moins","<","-",0}},
                                 {egal,{"egal","=",0}},
                                 {plus,{"plus",">","+",0}},
                                 {abandon_maitre,{"abandonner","quitter","abort","quit",0}},
                                 {0,{0}}};

const char* resultat_plus_ou_moins_label(resultat_plus_ou_moins resultat){
    switch(resultat){
      case moins:          return "moins grand que";
      case egal:           return "egal au";
      case plus:           return "plus grand que";
      case abandon_joueur: return "le joueur abandonne";
      case abandon_maitre: return "le maitre abandonne";
      case erreur_maitre:  return "le maitre a fait une erreur";
    }
}


typedef struct joueur_plus_ou_moins{
    void* data;
    void (*debut)(struct joueur_plus_ou_moins* joueur,int min,int max);
    // Appelée au début du jeu pour informer le joueur des limites.
    int (*jouer)(struct joueur_plus_ou_moins* joueur);
    // Appelée pour permettre au joueur de faire son choix.
    // Note: un résutat négatif indique l'abandon.
    void (*resultat)(struct joueur_plus_ou_moins* joueur,int choix,resultat_plus_ou_moins resultat);
    // Appelée pour indiquer au joueur le résultat de son choix.
    void (*fin)(struct joueur_plus_ou_moins* joueur,resultat_plus_ou_moins resultat_final);
    // Appelée pour signaler la fin du jeu au joueur.
} joueur_plus_ou_moins_t;

typedef struct maitre_plus_ou_moins{
    void* data;
    void (*debut)(struct maitre_plus_ou_moins* maitre,int min,int max);
    // Appelée au début du jeu pour informer le maitre des limites.
    int (*choisir_un_nombre)(struct maitre_plus_ou_moins* maitre);
    // Appelée pour permettre au maitre de choisir un nombre secret (dans les limites).
    // Note: un résutat négatif indique l'abandon.
    //       un résultat hors limite termine avec erreur_maitre.
    resultat_plus_ou_moins (*essai)(struct maitre_plus_ou_moins* maitre,int choix);
    // Appelée pour que le maitre indique le résultat du choix.
    // Si le maitre se trompe, le jeu se fini avec comme résultat erreur_maitre.
    void (*fin)(struct maitre_plus_ou_moins* maitre,resultat_plus_ou_moins resultat_final);
    // Appelée pour signaler la fin du jeu au maitre.
} maitre_plus_ou_moins_t;

typedef struct {
    int min;
    int max;
    maitre_plus_ou_moins_t* maitre;
    joueur_plus_ou_moins_t* joueur;
    int nombre_mystere;
} plus_ou_moins_t;


plus_ou_moins_t* nouveau_jeu_plus_ou_moins(int min,int max,
                                           maitre_plus_ou_moins_t* maitre,
                                           joueur_plus_ou_moins_t* joueur){
    assert(min<max);
    plus_ou_moins_t* jeu=check_not_null(malloc(sizeof(*jeu)),EX_OSERR,"Out of Memory");
    jeu->min=min;
    jeu->max=max;
    jeu->maitre=maitre;
    jeu->joueur=joueur;
    return jeu;
}

resultat_plus_ou_moins plus_ou_moins_essayer_nombre(int nombre_mystere,int essai){
    if(essai<0){
        return abandon_joueur;
    }else if(nombre_mystere<essai){
        return moins;
    }else if(nombre_mystere>essai){
        return plus;
    }else{
        return egal;
    }
}

void plus_ou_moins_jouer(plus_ou_moins_t* jeu){
    resultat_plus_ou_moins resultat;
    maitre_plus_ou_moins_t* maitre=jeu->maitre;
    joueur_plus_ou_moins_t* joueur=jeu->joueur;
    maitre->debut(maitre,jeu->min,jeu->max);
    joueur->debut(joueur,jeu->min,jeu->max);
    jeu->nombre_mystere=maitre->choisir_un_nombre(maitre);
    if(jeu->nombre_mystere<0){
        resultat=abandon_maitre;
    }else if((jeu->nombre_mystere<jeu->min)
             ||(jeu->max<jeu->nombre_mystere)){
        resultat=erreur_maitre;
    }else{
        do{
            int choix=joueur->jouer(joueur);
            int resultat_attendu=plus_ou_moins_essayer_nombre(jeu->nombre_mystere,choix);
            resultat=maitre->essai(maitre,choix);
            if((resultat!=abandon_maitre)&&(resultat!=resultat_attendu)){
                resultat=erreur_maitre;
            }
            joueur->resultat(joueur,choix,resultat);
        }while((resultat==moins)||(resultat==plus));
    }
    maitre->fin(maitre,resultat);
    joueur->fin(joueur,resultat);
}



////////////////////////////////////////////////////////////////////////
//
// En ayant ainsi un jeu abstrait, on peut l'utiliser avec divers
// joueurs, lesquels peuvent avoir différent interfaces utilisateurs.
//
// On commence par un maitre automatique
//
////////////////////////////////////////////////////////////////////////

typedef struct {
    int min;
    int max;
    int secret;
} auto_maitre_data_t;

void auto_maitre_debut(maitre_plus_ou_moins_t* maitre,int min,int max){
    // Appelée au début du jeu pour informer le maitre des limites.
    auto_maitre_data_t* data=check_not_null(malloc(sizeof(*data)),EX_OSERR,"Out of Memory");
    data->min=min;
    data->max=max;
    maitre->data=data;
    printf("jeu :    Choisissez un entier mystere entre %d et %d\n",data->min,data->max);
}

int auto_maitre_choisir_un_nombre(maitre_plus_ou_moins_t* maitre){
    // Appelée pour permettre au maitre de choisir un nombre secret (dans les limites).
    // Note: un résutat négatif indique l'abandon.
    //       un résultat hors limite termine avec erreur_maitre.
    auto_maitre_data_t* data=maitre->data;
    assert(data->min!=data->max);
    data->secret=(random()%(data->max+1-data->min))+data->min;
    printf("maitre : J'ai choisi mon nombre mystere.\n");
    return data->secret;
}

resultat_plus_ou_moins auto_maitre_essai(maitre_plus_ou_moins_t* maitre,int essai){
    // Appelée pour que le maitre indique le résultat du choix.
    // Si le maitre se trompe, le jeu se fini avec comme résultat erreur_maitre.
    auto_maitre_data_t* data=maitre->data;
    resultat_plus_ou_moins resultat=plus_ou_moins_essayer_nombre(data->secret,essai);
    if(essai<0){
        printf("maitre : %s !\n",resultat_plus_ou_moins_label(resultat));
    }else{
        printf("maitre : le nombre mystere est %s le nombre choisi.\n",
               resultat_plus_ou_moins_label(resultat));
    }
    return resultat;
}

void auto_maitre_fin(unused maitre_plus_ou_moins_t* maitre,resultat_plus_ou_moins resultat_final){
    // Appelée pour signaler la fin du jeu au maitre.
    switch(resultat_final){
      case egal:
          printf("maitre : Bravo, vous avez trouve le nombre mystere !!!\n");
          break;
      case abandon_joueur:
      case abandon_maitre:
      case erreur_maitre:
          printf("jeu :    %s !\n",resultat_plus_ou_moins_label(resultat_final));
          break;
      default:
          should_not_occur();
          break;
    }
}


maitre_plus_ou_moins_t* nouveau_auto_maitre(){
    maitre_plus_ou_moins_t* maitre=check_not_null(malloc(sizeof(*maitre)),EX_OSERR,"Out of Memory");
    maitre->debut              = &auto_maitre_debut;
    maitre->choisir_un_nombre  = &auto_maitre_choisir_un_nombre;
    maitre->essai              = &auto_maitre_essai;
    maitre->fin                = &auto_maitre_fin;
    return maitre;
}


////////////////////////////////////////////////////////////////////////
//
// Maintenant, on représente un maitre humain utilisant un interface
// utilisateur CLI:
//
////////////////////////////////////////////////////////////////////////

typedef struct {
    int min;
    int max;
    int secret;
}  cli_maitre_data_t;


void cli_maitre_debut(maitre_plus_ou_moins_t* maitre,int min,int max){
    // Appelée au début du jeu pour informer le maitre des limites.
    cli_maitre_data_t* data=check_not_null(malloc(sizeof(*data)),EX_OSERR,"Out of Memory");
    data->min=min;
    data->max=max;
    maitre->data=data;
}

int cli_maitre_choisir_un_nombre(maitre_plus_ou_moins_t* maitre){
    cli_maitre_data_t* data=maitre->data;
    printf("jeu :    Choisissez un entier mystere entre %d et %d\n",data->min,data->max);
    data->secret=demander_un_nombre("maitre");
    clear_screen();
    return data->secret;
}

resultat_plus_ou_moins cli_maitre_essai(unused maitre_plus_ou_moins_t* maitre,int choix){
    // Appelée pour que le maitre indique le résultat du choix.
    // Si le maitre se trompe, le jeu se fini avec comme résultat erreur_maitre.
    if(choix<0){
        return abandon_joueur;
    }
    printf("jeu :    le joueur a choisi %d; est-ce moins, plus ou egal au nombre mystere?\n",choix);
    resultat_plus_ou_moins resultat=demander("maitre : ",options_comparaison);
    return resultat;
}

void cli_maitre_fin(unused maitre_plus_ou_moins_t* maitre,resultat_plus_ou_moins resultat_final){
    // Appelée pour signaler la fin du jeu au maitre.
    auto_maitre_fin(maitre,resultat_final);
}


maitre_plus_ou_moins_t* nouveau_cli_maitre(){
    maitre_plus_ou_moins_t* maitre=check_not_null(malloc(sizeof(*maitre)),EX_OSERR,"Out of Memory");
    maitre->debut             = &cli_maitre_debut;
    maitre->choisir_un_nombre = &cli_maitre_choisir_un_nombre;
    maitre->essai             = &cli_maitre_essai;
    maitre->fin               = &cli_maitre_fin;
    return maitre;
}


////////////////////////////////////////////////////////////////////////
//
// Ici on représente un joueur humain utilisant un interface
// utilisateur CLI:
//
////////////////////////////////////////////////////////////////////////


void indiquer_ordre(const char* ordre,int choix){
    printf("jeu :    Le nombre mystere est %s %d!\n",ordre,choix);
}

typedef struct {
    int min;
    int max;
    char* invite;
}  cli_joueur_data_t;


void cli_joueur_debut(joueur_plus_ou_moins_t* joueur,int min,int max){
    // Appelée au début du jeu pour informer le joueur des limites.
    cli_joueur_data_t* data=check_not_null(malloc(sizeof(*data)),EX_OSERR,"Out of Memory");
    data->min=min;
    data->max=max;
    int size=1+snprintf(0,0,"Devinez un entier entre %d et %d",data->min,data->max);
    data->invite=check_not_null(malloc(size),EX_OSERR,"Out of Memory");
    snprintf(data->invite,size,"Devinez un entier entre %d et %d",data->min,data->max);
    joueur->data=data;
}

int cli_joueur_jouer(joueur_plus_ou_moins_t* joueur){
    // Appelée pour permettre au joueur de faire son choix.
    // Note: un résutat négatif indique l'abandon.
    cli_joueur_data_t* data=joueur->data;
    return(demander_un_nombre(data->invite));
}


void cli_joueur_resultat(unused joueur_plus_ou_moins_t* joueur,unused int choix,resultat_plus_ou_moins resultat){
    // Appelée pour indiquer au joueur le résultat de son choix.
    if(resultat<=plus){
        indiquer_ordre(resultat_plus_ou_moins_label(resultat),choix);
    }
}

void cli_joueur_fin(unused joueur_plus_ou_moins_t* joueur,resultat_plus_ou_moins resultat_final){
    // Appelée pour signaler la fin du jeu au joueur.
    if(resultat_final==egal){
        printf("joueur : Bravo, vous avez trouve le nombre mystere !!!\n");
    }
}


joueur_plus_ou_moins_t* nouveau_cli_joueur(){
    joueur_plus_ou_moins_t* joueur=check_not_null(malloc(sizeof(*joueur)),EX_OSERR,"Out of Memory");
    joueur->debut    = &cli_joueur_debut;
    joueur->jouer    = &cli_joueur_jouer;
    joueur->resultat = &cli_joueur_resultat;
    joueur->fin      = &cli_joueur_fin;
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
} auto_joueur_data_t;

void auto_joueur_debut(joueur_plus_ou_moins_t* joueur,int min,int max){
    // Appelée au début du jeu pour informer le joueur des limites.
    auto_joueur_data_t* data=check_not_null(malloc(sizeof(*data)),EX_OSERR,"Out of Memory");
    data->min=min;
    data->max=max;
    joueur->data=data;
    printf("jeu :    Devinez un entier entre %d et %d\n",data->min,data->max);
}

int auto_joueur_jouer(joueur_plus_ou_moins_t* joueur){
    // Appelée pour permettre au joueur de faire son choix.
    // Note: un résutat négatif indique l'abandon.
    auto_joueur_data_t* data=joueur->data;
    assert(data->min!=data->max);
    int essai=(data->min+data->max+1)/2;
    printf("joueur : Est-ce %d ?\n",essai);
    return essai;
}

void auto_joueur_resultat(unused joueur_plus_ou_moins_t* joueur,unused int choix,resultat_plus_ou_moins resultat){
    // Appelée pour indiquer au joueur le résultat de son choix.
    auto_joueur_data_t* data=joueur->data;
    indiquer_ordre(resultat_plus_ou_moins_label(resultat),choix);
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
          printf("joueur : J'ai trouvé %d !\n",choix);
          break;
      default:
          should_not_occur();
          break;
    }
}

void auto_joueur_fin(unused joueur_plus_ou_moins_t* joueur,resultat_plus_ou_moins resultat_final){
    // Appelée pour signaler la fin du jeu au joueur.
    if(resultat_final==egal){
        printf("jeu :    Bravo, vous avez trouve le nombre mystere !!!\n");
    }else{
        printf("jeu :    Lacheur !\n");
    }
}


joueur_plus_ou_moins_t* nouveau_auto_joueur(){
    joueur_plus_ou_moins_t* joueur=check_not_null(malloc(sizeof(*joueur)),EX_OSERR,"Out of Memory");
    joueur->debut    = &auto_joueur_debut;
    joueur->jouer    = &auto_joueur_jouer;
    joueur->resultat = &auto_joueur_resultat;
    joueur->fin      = &auto_joueur_fin;
    return joueur;
}

////////////////////////////////////////////////////////////////////////
//
// Testons les deux joueurs:
//
////////////////////////////////////////////////////////////////////////

void test_plus_ou_moins(int min,int max){
    printf("\n\nJoueur automatique\n\n");
    plus_ou_moins_jouer(nouveau_jeu_plus_ou_moins(min,max,nouveau_auto_maitre(),nouveau_auto_joueur()));
    printf("\n\nJoueur humain (c'est a vous!)\n\n");
    plus_ou_moins_jouer(nouveau_jeu_plus_ou_moins(min,max,nouveau_auto_maitre(),nouveau_cli_joueur()));
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

typedef maitre_plus_ou_moins_t* (*nouveau_maitre_pr)();
typedef joueur_plus_ou_moins_t* (*nouveau_joueur_pr)();



void plusieurs_plus_ou_moins(int min,int max,
                             nouveau_maitre_pr maitre_createur,
                             nouveau_joueur_pr joueur_createur){
    int encore=0;
    do{
        plus_ou_moins_jouer(nouveau_jeu_plus_ou_moins(min,max,
                                                      maitre_createur(),
                                                      joueur_createur()));
        encore=demander("Voulez vous essayer encore",options_booleennes);
    }while(encore>0);
}



void usage(const char* pname){
    size_t len=strlen(pname);
    char* spaces=check_not_null(malloc(len+1),EX_OSERR,"Out of Memory");
    memset(spaces,' ',len);spaces[len]='\0';
    printf("%s usage:\n\n",pname);
    printf("\t%s [ test | menu | maitre=auto|cli \\\n",pname);
    printf("\t%s | joueur=auto|cli | plusieurs] \\\n",spaces);
    printf("\t%s | niveau=1|2|3|4|5|6 ]\n\n",spaces);
}

typedef struct {
    int plusieurs;
    int niveau;
    nouveau_maitre_pr maitre_pr;
    nouveau_joueur_pr joueur_pr;
} configuration_t;

configuration_t* configuration_nouveau(){
    configuration_t* configuration=check_not_null(malloc(sizeof(*configuration)),
                                                  EX_OSERR,"Out of Memory");
    configuration->plusieurs=0;
    configuration->niveau=1;
    configuration->maitre_pr=&nouveau_auto_maitre;
    configuration->joueur_pr=&nouveau_cli_joueur;
    return configuration;
}

const char* boolean_label(int boolean){
    return boolean?"oui":"non";
}

const char* cli_label(int cli){
    return cli?"cli":"auto";
}

option_t options_configuration[]={ {0,{"v","ok","valider",0}},
                                       {1,{"p","plusieurs",0}},
                                       {2,{"m","maitre",0}},
                                       {3,{"j","joueur",0}},
                                       {4,{"n","niveau",0}},
                                       {0,{0}}};

void configuration_menu(configuration_t* configuration){
    int maitre_cli=(configuration->maitre_pr==&nouveau_cli_maitre);
    int joueur_cli=(configuration->joueur_pr==&nouveau_cli_joueur);
    while(1){
        printf("\nConfiguration:\n");
        printf("  (p) plusieurs parties : %s\n",
               boolean_label(configuration->plusieurs));
        printf("  (n) niveau            : %d\n",configuration->niveau);
        printf("  (m) maitre            : %s\n",cli_label(maitre_cli));
        printf("  (j) joueur            : %s\n",cli_label(joueur_cli));
        printf("  (v) valider\n");
        printf("\n");
        switch(demander("Choix",options_configuration)){
          case 0:
              return;
          case 1:
              configuration->plusieurs=(!configuration->plusieurs);
              break;
          case 2:
              maitre_cli=(!maitre_cli);
              configuration->maitre_pr=(maitre_cli
                                        ?&nouveau_cli_maitre
                                        :&nouveau_auto_maitre);
              break;
          case 3:
              joueur_cli=(!joueur_cli);
              configuration->joueur_pr=(joueur_cli
                                        ?&nouveau_cli_joueur
                                        :&nouveau_auto_joueur);
              break;
          case 4:{
              int niveau=demander_un_nombre("Niveau (entre 1 et 6 inclus): ");
              if((niveau<1)||(6<niveau)){
                  fprintf(stderr,"Niveau %d invalide\n",niveau);
              }else{
                  configuration->niveau=niveau;
              }
              break;
          }
        }
    }
}


int main(const int argc,const char* argv[]){
    initialiser();
    configuration_t* configuration=configuration_nouveau();
    int i;
    int max=100;
    for(i=1;i<argc;++i){
        if(strcmp("test",argv[i])==0){
            test_plus_ou_moins(1,max);
            return 0;
        }else if(strcmp("menu",argv[i])==0){
            configuration_menu(configuration);
        }else if(strcmp("maitre=auto",argv[i])==0){
            configuration->maitre_pr=&nouveau_auto_maitre;
        }else if(strcmp("maitre=cli",argv[i])==0){
            configuration->maitre_pr=&nouveau_cli_maitre;
        }else if(strcmp("joueur=auto",argv[i])==0){
            configuration->joueur_pr=&nouveau_auto_joueur;
        }else if(strcmp("joueur=cli",argv[i])==0){
            configuration->joueur_pr=&nouveau_cli_joueur;
        }else if(strncmp("niveau=",argv[i],strlen("niveau="))==0){
            int nouveau_niveau=0;
            sscanf(argv[i]+strlen("niveau="),"%20i",&nouveau_niveau);
            if((nouveau_niveau<0)||(6<nouveau_niveau)){
                fprintf(stderr,"Invalid level: %d in argument %s\n",
                        nouveau_niveau,argv[i]);
                usage(basename(string_copy(argv[0])));
                exit(EX_USAGE);
            }else{
                configuration->niveau=nouveau_niveau;
            }
        }else if(strcmp("plusieurs",argv[i])==0){
            configuration->plusieurs=1;
        }else{
            fprintf(stderr,"Invalid argument: %s\n",argv[i]);
            usage(basename(string_copy(argv[0])));
            exit(EX_USAGE);
        }
    }
    max=10*expi(10,configuration->niveau);
    if(configuration->plusieurs){
        plusieurs_plus_ou_moins(1,max,
                                configuration->maitre_pr,
                                configuration->joueur_pr);
    }else{
        plus_ou_moins_jouer(nouveau_jeu_plus_ou_moins(1,max,
                                                      configuration->maitre_pr(),
                                                      configuration->joueur_pr()));
    }
    return 0;
}

// PS: It is assumed to be compiled with BoehmGC.
// cc -g3 -ggdb  -L/opt/local/lib -lgc  plus-ou-moins-2.c   -o plus-ou-moins-2

/* THE END */
