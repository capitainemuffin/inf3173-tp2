#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define QTE_BAGUETTES_REQUISES_MANGER 2
#define QTE_PHILOSOPHES 5
#define QTE_BAGUETTES 5

/**
* Le nombre de fois qu'un philosophe doit faire chaque action
**/
#define QTE_MANGER_MAX 5
#define QTE_PENSER_MAX 5

/**
* Le temps maximum et minimum en secondes
* qu'un philosophe peut prendre pour une action
**/
#define TEMPS_MAX 3
#define TEMPS_MIN 1

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
int nbr_baguettes_disponibles = QTE_BAGUETTES;
int code = 0;

/**
* Représente les actions que peut faire un philosophe
**/
typedef enum {
    PENSER, MANGER
} Action;

/**
* Représente un philosophe 
**/
typedef struct {
    int numero;
    Action action;
    FILE *fp;
} Philosophe;

/**
* Le constructeur d'un philosophe 
* Renvoi un pointeur vers une structure <Philosophe>
**/
Philosophe *initPhilosophe(int i, FILE *fp) {
    Philosophe *p = malloc(sizeof(Philosophe));
    p->action = PENSER;
    p->numero = i;
    p->fp = fp;
    return p;
}

/**
* Destructeur d'un philosophe
* Libère la mémoire allouée à la structure <Philosophe> p
**/
void freePhilosophe(Philosophe *p) {
    free(p);
}

void pickup_forks() {
    pthread_mutex_lock(&mutex);
    while (nbr_baguettes_disponibles < QTE_BAGUETTES_REQUISES_MANGER) {
        pthread_cond_wait(&condition_var, &mutex);
    }
    pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&mutex);

    pthread_mutex_lock(&mutex);
    nbr_baguettes_disponibles -= QTE_BAGUETTES_REQUISES_MANGER;
    pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&mutex);
}

void return_forks() {
    pthread_mutex_lock(&mutex);
    nbr_baguettes_disponibles += QTE_BAGUETTES_REQUISES_MANGER;
    pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&mutex);
}

void ecrire_ligne(Philosophe *p) {

    pthread_mutex_lock(&mutex);
    code++;
    system("clear");

    printf("Le dîner des philosophes se déroule ... %d%%\n", code * 2);
    if (p->action == PENSER) {
        fprintf(p->fp, "%d\tPhilosophe %d\tpense\n", code, p->numero);
    } else {
        fprintf(p->fp, "%d\tPhilosophe %d\tmange\n", code, p->numero);
    }
    pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&mutex);
}

void *faire_une_action(void *philo) {

    Philosophe *p = philo;
    int qte_penser = 0;
    int qte_manger = 0;

    while (qte_penser < QTE_PENSER_MAX || qte_manger < QTE_MANGER_MAX) {

        if (p->action == MANGER) {
            return_forks();
            p->action = PENSER;
            qte_penser++;
        } else {
            pickup_forks();
            p->action = MANGER;
            qte_manger++;
        }

        ecrire_ligne(p);

        // Temps pris pour faire l'action
        int temps = (rand() % (TEMPS_MAX + 1 - TEMPS_MIN)) + TEMPS_MIN;
#ifdef _WIN32
        Sleep(temps * 1000);
#else
        sleep(temps);
#endif

    }

    pthread_exit(NULL);

}

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
        perror("Le champ « nom » est verrouillé, veuillez réessayer plus tard : ");
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
        perror("Une erreur est survenue lors du déblocage du champ nom : ");
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
        perror("Le fichier « résultat » est verrouillé, veuillez réessayer plus tard : ");
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
        perror("Une erreur est survenue lors du déblocage du fichier : ");
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
    char nom[64];
    char action[64];
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
        perror("L’enregistrement que vous voulez accéder est verrouillé, veuillez réessayer plus tard : ");
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
        perror("Une erreur est survenue lors du déblocage de l'enregistrement : ");
        exit(-4);
    }

}

int main() {

    // Créer le fichier
    FILE *fp = fopen("resultat.txt", "wb+");
    int choix = -1;

    if (fp == NULL) {
        perror("Erreur lors de la création du fichier \'resultat.txt\' : ");
        exit(-3);
    }

    // L'en-tête du fichier
    fprintf(fp, "Code\tNom\t\tAction\n");

    pthread_t threads[QTE_PHILOSOPHES];
    Philosophe *philosophes[QTE_PHILOSOPHES];

    for (int i = 0; i < QTE_PHILOSOPHES; i++) {

        Philosophe *p = initPhilosophe(i, fp);
        philosophes[i] = p;

        if (pthread_create(&threads[i], NULL, &faire_une_action,
                           philosophes[i]) != 0) {
            perror("Une erreur est survenue à l'ouverture de thread : ");
            exit(-1);
        }
    }

    //Attendre que tous les threads se terminent
    // et libérer les espaces mémoires des philosophes
    for (int i = 0; i < QTE_PHILOSOPHES; i++) {

        if (pthread_join(threads[i], NULL) != 0) {
            perror("Une erreur est survenue à l'attente de terminaison du thread : ");
            exit(-2);
        }

        freePhilosophe(philosophes[i]);
    }
    fclose(fp);
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
