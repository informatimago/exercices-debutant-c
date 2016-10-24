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

    … Et il vous indique si le nombre mystère est supérieur ou
    inférieur.

    Et ainsi de suite, jusqu'à ce que vous trouviez le nombre mystère.

Le but du jeu, bien sûr, est de trouver le nombre mystère en un
minimum de coups.

Faites un compteur de « coups ». Ce compteur devra être une variable
que vous incrémenterez à chaque fois que vous passez dans la
boucle. Lorsque l'utilisateur a trouvé le nombre mystère, vous lui
direz « Bravo, vous avez trouvé le nombre mystère en 8 coups » par
exemple.

Lorsque l'utilisateur a trouvé le nombre mystère, le programme
s'arrête. Pourquoi ne pas demander s'il veut faire une autre partie ?
Si vous faites ça, il vous faudra faire une boucle qui englobera la
quasi-totalité de votre programme. Cette boucle devra se répéter TANT
QUE l'utilisateur n'a pas demandé à arrêter le programme. Je vous
conseille de rajouter une variable booléenne continuerPartie
initialisée à 1 au départ. Si l'utilisateur demande à arrêter le
programme, vous mettrez la variable à 0 et le programme s'arrêtera.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


void initialiser(){
    srandom(clock());
}

int choisir_un_nombre_aleatoire_dans_l_intervale(int min,int max){
    return random()%(max-min+1)+min;
}

int demander_un_nombre(const char* invite){
    printf("%s : ",invite);
    fflush(stdout); // tres important!
    int nombre=0;
    scanf("%12i",&nombre);
    return nombre;
}

void indiquer_ordre(const char* ordre){
    printf("C'est %s !\n",ordre);
}

void plus_ou_moins(){
    int nombre_secret=choisir_un_nombre_aleatoire_dans_l_intervale(1,100);
    int nombre_teste=0;
    int compte_essai=0;
    do{
        compte_essai++;
        nombre_teste=demander_un_nombre("Devinez un entier entre 1 et 100");
        if(nombre_teste<nombre_secret){
            indiquer_ordre("plus");
        }else if(nombre_teste>nombre_secret){
            indiquer_ordre("moins");
        }else{
            indiquer_ordre("egal");
        }
    }while(nombre_secret!=nombre_teste);
    printf("Bravo, vous avez trouve le nombre mystere en %d essais !!!\n",compte_essai);
}

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

int main(){
    initialiser();
    int encore=0;
    do{
        plus_ou_moins();
        encore=ask("Voulez vous essayer encore","non","oui");
    }while(encore>0);
    return 0;
}
