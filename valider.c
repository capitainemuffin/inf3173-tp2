#include <stdio.h>


/**
* Affiche les résultats.
* Aucun verrouillage n’est fait.
**/
void consulter_resultat(FILE *fp) {
    rewind(fp);
    char c;
    c = fgetc(fp);

    while (c != EOF) {
        printf("%c", c);
        c = fgetc(fp);
    }
}

/**
* Demande le code de philosophe dont on veut modifier son nom.
* Verouille le champ "nom" qui est associé à ce code.
**/
void modifier_nom() {

}

/**
* Supprimer le nom d’un philosophe.
* Verrouille le fichier en entier.
**/
void supprimer_nom() {

}

/**
* Demande le code du philosophe dont on veut modifier l'action.
* Verouille l’enregistrement (la ligne) qui est associée à ce code.
**/
void modifier_action() {

}

/**
* Le programme se ferme en affichant le contenu de
* fichier « résultat ».
**/
void quitter(FILE *fp) {
    consulter_resultat(fp);
    fclose(fp);
    exit(0);
}

int main(){

}
    while (1) {

        printf("Choisissez l'une des options suivantes et appuyez sur entrée.\n");
        printf("[ 1 ] Consulter le résultat\n");
        printf("[ 2 ] Modifier le nom d'un philosophe\n");
        printf("[ 3 ] Supprimer le nom d'un philosophe\n");
        printf("[ 4 ] Modifier le nom et l'action d'un philosophe\n");
        printf("[ 5 ] Quitter");

        char line[256];
        fgets(line, sizeof(line), stdin);
        sscanf(line, "%d", &choix);
        switch (choix) {
            case 1 :
                consulter_resultat(fp);
                break;
            case 2 :
                modifier_nom();
                break;
            case 3 :
                supprimer_nom();
                break;
            case 4 :
                modifier_nom();
                modifier_action();
                break;
            case 5 :
                quitter(fp);
                break;
            default :
                printf("Choix invalide ! \n");
        }

}
