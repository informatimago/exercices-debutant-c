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
#include <stdlib.h>
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
    scanf("%i",&nombre);
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

int main(){
    initialiser();
    plus_ou_moins();
    return 0;
}
