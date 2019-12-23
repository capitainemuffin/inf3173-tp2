#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _WIN32
#include <Windows.h>
#else

#include <unistd.h>

#endif

#define QTE_BAGUETTES_REQUISES_MANGER 2
#define QTE_PHILOSOPHES 5
#define QTE_BAGUETTES 5

/**
* Le nombre de fois qu'un philosophe doit faire chaque action
**/
#define QTE_MANGER_MAX 5
#define QTE_PENSER_MAX 5

/**
* Le temps maximum et minimum en secondes qu'un philosophe peut prendre pour une action
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

#ifdef _WIN32
    system("@cls");
#else
    system("clear");
#endif

    printf("Le dîner des philosophes se déroule ... %d%%\n", code * 2);
    if (p->action == PENSER) {
        fprintf(p->fp, "%d 	Philosophe %d 	pense\n", code, p->numero);
    } else {
        fprintf(p->fp, "%d 	Philosophe %d 	mange\n", code, p->numero);
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
void modifier_nom(FILE* fp) {

    int fd = fileno(fp);

    // acquire exclusive lock for bytes in range [10; 15)
    // F_LOCK specifies blocking mode
    if (lockf(fd, F_LOCK, 5) == -1) {
        exit(1);
    }

    // release lock for bytes in range [10; 15)
    if (lockf(fd, F_ULOCK, 5) == -1) {
        exit(1);
    }
}

/**
* Supprimer le nom d’un philosophe. 
* Verrouille le fichier en entier.
**/
void supprimer_nom(FILE* fp) {
    int fd = fileno(fp);

}

/**
* Demande le code du philosophe dont on veut modifier l'action.
* Verouille l’enregistrement (la ligne) qui est associée à ce code.
**/
void modifier_nom_et_action(FILE* fp) {
    int fd = fileno(fp);

}

int main() {

    // Créer le fichier
    FILE *fp = fopen("resultat.txt", "w+");
    int choix = -1;

    if (fp == NULL) {
        perror("Error opening th file");
        exit(-3);
    }

    // L'en-tête du fichier
    fprintf(fp, "Code 	Nom 	 	Action\n");

    pthread_t threads[QTE_PHILOSOPHES];
    Philosophe *philosophes[QTE_PHILOSOPHES];

    for (int i = 0; i < QTE_PHILOSOPHES; i++) {

        Philosophe *p = initPhilosophe(i + 1, fp);
        philosophes[i] = p;

        if (pthread_create(&threads[i], NULL, &faire_une_action,
                           philosophes[i]) != 0) {
            perror("Une erreur est survenue à l'ouverture de thread.");
            exit(-1);
        }
    }

    for (int i = 0; i < QTE_PHILOSOPHES; i++) {

        if (pthread_join(threads[i], NULL) != 0) {
            perror("Une erreur est survenue à l'attente de terminaison du thread.");
            exit(-2);
        }

        freePhilosophe(philosophes[i]);
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
                modifier_nom(fp);
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
