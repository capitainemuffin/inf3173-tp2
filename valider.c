#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
    printf("\n");
}

/**
* Demande le code de philosophe dont on veut modifier son nom.
* Verouille le champ "nom" qui est associé à ce code.
**/
void modifier_nom(FILE *fp) {

    int fd = fileno(fp);
    int ligne = 0;
    int idx_debut_nom = 0;
    int idx_fin_nom = 0;
    int choix = 0;
    char line[256];
    char c;

    //Valider que l'entrée est un chiffre.
    do {
        printf("Veuillez entrer le code de philosophe à modifier\n");
        fgets(line, sizeof(line), stdin);
        choix = atoi(line);

        if (choix == 0) {
            printf("Choix invalide !\n");
        }
    } while (choix == 0);

    rewind(fp);

    // Arriver à la bonne ligne
    while ((c = fgetc(fp)) != EOF && ligne != choix) {
        idx_debut_nom++;
        idx_fin_nom++;
        if (c == '\n') ligne++;
    }
    idx_debut_nom++;

    // Avoir l'index du début du nom
    while ((c = fgetc(fp)) != EOF && c != '\t') {
        idx_debut_nom++;
        idx_fin_nom++;
    }
    idx_debut_nom++;

    // Avoir l'index de fin du nom
    while ((c = fgetc(fp)) != EOF && c != '\t') idx_fin_nom++;
    idx_fin_nom++;

    //Bloquer le champ nom
    fseek(fp, idx_debut_nom, SEEK_SET);
    int taille = idx_fin_nom - idx_debut_nom + 1;

    if (lockf(fd, F_TLOCK, taille) == -1) {
        perror("Le champ « nom » est verrouillé, veuillez réessayer plus tard.");
        return;
    }

    printf("Veuillez entrer le nouveau nom du philosophe.\n");
    fgets(line, sizeof(line), stdin);
    strtok(line, "\n");

    if (taille < strlen(line)) {
        printf("Désolé, la taille maximale (%d) pour le nom est dépassée (%lu).\n",
               taille, strlen(line));
    } else {
        fwrite(line, taille, 1, fp);
        printf("Nom modifié avec succès.\n");
    }

    // Débloquer le champ nom
    fseek(fp, idx_debut_nom, SEEK_SET);
    if (lockf(fd, F_ULOCK, taille) == -1) {
        perror("Une erreur est survenue lors du déblocage du champ nom.");
        exit(-5);
    }
}

/**
* Supprimer le nom d’un philosophe. 
* Verrouille le fichier en entier.
**/
void supprimer_nom(FILE *fp) {

    int fd = fileno(fp);
    int ligne = 0;
    int choix = 0;
    char line[256];
    char c;

    //bloquer le fichier
    fseek(fp, 0, SEEK_END);
    int taille_fichier = ftell(fp);
    rewind(fp);
    if (lockf(fd, F_TLOCK, taille_fichier) == -1) {
        perror("Le fichier « résultat » est verrouillé, veuillez réessayer plus tard.");
        return;
    }

    //Valider que l'entrée est un chiffre.
    do {
        printf("Veuillez entrer le code de philosophe à supprimer\n");
        fgets(line, sizeof(line), stdin);
        choix = atoi(line);

        if (choix == 0) {
            printf("Choix invalide !\n");
        }
    } while (choix == 0);

    rewind(fp);

    // Arriver au début de la bonne ligne
    while ((c = fgetc(fp)) != EOF && ligne != choix) if (c == '\n') ligne++;
    fseek(fp, -1, SEEK_CUR);

    // Remplacer le texte par des espaces
    while (c != EOF && c != '\n') {
        if (c != '\t') {
            fputc(' ', fp);
        } else {
            fputc('\t', fp);
        }
        c = fgetc(fp);
        fseek(fp, -1, SEEK_CUR);
    }

    printf("Nom supprimé avec succès.\n");

    //Débloquer fichier
    rewind(fp);
    if (lockf(fd, F_ULOCK, taille_fichier) == -1) {
        perror("Une erreur est survenue lors du déblocage du fichier.");
        exit(-4);
    }

}

/**
* Demande le code du philosophe dont on veut modifier l'action.
* Verouille l’enregistrement (la ligne) qui est associée à ce code.
**/
void modifier_nom_et_action(FILE *fp) {
    int fd = fileno(fp);
    int ligne = 0;
    int choix = 0;
    int idx_debut_ligne = 0;
    int idx_fin_ligne = 0;
    int taille = 0;
    char nom[256];
    char action[256];
    char line[256];
    char c;

    //Valider que l'entrée est un chiffre.
    do {
        printf("Veuillez entrer le code de philosophe à modifier\n");
        fgets(line, sizeof(line), stdin);
        strtok(action, "\n");

        choix = atoi(line);

        if (choix == 0) {
            printf("Choix invalide !\n");
        }
    } while (choix == 0);

    rewind(fp);

    // Arriver au début de la bonne ligne
    while ((c = fgetc(fp)) != EOF && ligne != choix) if (c == '\n') ligne++;
    fseek(fp, -1, SEEK_CUR);

    idx_debut_ligne = ftell(fp);
    while(c != '\n') c = fgetc(fp);
    fseek(fp, -1, SEEK_CUR);

    idx_fin_ligne = ftell(fp);
    taille = idx_fin_ligne - idx_debut_ligne;

    // Bloquer la ligne
    fseek(fp, idx_debut_ligne, SEEK_SET);
    if (lockf(fd, F_TLOCK, taille) == -1) {
        perror("L’enregistrement que vous voulez accéder est verrouillé, veuillez réessayer plus tard.");
        return;
    }

    printf("Veuillez entrer le nouveau nom du philosophe.\n");
    fgets(nom, sizeof(nom), stdin);
    strtok(nom, "\n");

    printf("Veuillez entrer la nouvelle action du philosophe.\n");
    fgets(action, sizeof(action), stdin);
    strtok(action, "\n");

    sprintf(line, "%d\t%s\t%s\n", choix, nom, action);
    fwrite(line, taille, 1, fp);
    printf("Nom et action modifiées avec succès.\n");

    //Débloquer la ligne
    fseek(fp, idx_debut_ligne, SEEK_SET);
    if (lockf(fd, F_ULOCK, taille) == -1) {
        perror("Une erreur est survenue lors du déblocage de l'enregistrement.");
        exit(-4);
    }

}

int main(){

    FILE *fp;
    int choix = -1;

    if((fp = fopen("resultat.txt", "rb+")) == NULL){
    	perror("Erreur lors de l'ouverture du fichier \'resultat.txt\' : ");
    	exit(-1);
    }

	while (1) {

        printf("Choisissez l'une des options suivantes et appuyez sur entrée.\n");
        printf("[ 1 ] Consulter le résultat\n");
        printf("[ 2 ] Modifier le nom d'un philosophe\n");
        printf("[ 3 ] Supprimer le nom d'un philosophe\n");
        printf("[ 4 ] Modifier le nom et l'action d'un philosophe\n");
        printf("[ 5 ] Quitter\n");

        char line[256];
        fgets(line, sizeof(line), stdin);
        sscanf(line, "%d", &choix);

        switch (choix) {
            case 1 :
                consulter_resultat(fp);
                break;
            case 2 :
                modifier_nom(fp);
                break;
            case 3 :
                supprimer_nom(fp);
                break;
            case 4 :
                modifier_nom_et_action(fp);
                break;
            case 5 :
                consulter_resultat(fp);
                fclose(fp);
                exit(0);
            default :
                printf("Choix invalide !\n");
        }

    }

}
