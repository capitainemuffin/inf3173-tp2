#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#define QTE_BAGUETTES_REQUISES_MANGER 2;
#define QTE_PHILOSOPHES 5
#define QTE_BAGUETTES 5

/**
* Le nombre de fois qu'un philosophe doit faire chaque action
**/
#define QTE_MANGER_MAX 5
#define QTE_PENSER_MAX 5

/**
* Le temps maximum et minimum qu'un philosophe peut prendre pour une action
**/
#define TEMPS_MAX 3
#define TEMPS_MIN 1

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t nbr_baguettes_disponibles = 5;

/**
* Représente les actions que peut faire un philosophe
**/
typedef enum {
	MANGER, PENSER
} Action;

/**
* Représente un philosophe 
**/
typedef struct {
	int qte_baguettes_en_main;
	Action action;
	int qte_manger;
	int qte_penser;
} Philosophe;

/**
* Le constructeur d'un philosophe 
* Renvoi un pointeur vers une structure <Philosophe>
**/
Philosophe* initPhilosophe(){

	Philosophe* p = malloc(sizeof(Philosophe));
	p->action = NULL;
	return p;

}

/**
* Destructeur d'un philosophe
* Libère la mémoire allouée à la structure <Philosophe> p
**/
void freePhilosophe(Philosophe* p){

	free(p);

}

void pickup_forks(){

	pthread_mutex_lock(&mutex);
	nbr_baguettes_disponibles -= QTE_BAGUETTES_REQUISES_MANGER;
	pthread_mutex_unlock(&mutex);

}

void return_forks(){

	pthread_mutex_lock(&mutex);
	nbr_baguettes_disponibles += QTE_BAGUETTES_REQUISES_MANGER;
	pthread_mutex_unlock(&mutex);

}



void* faire_une_action(void* philo){
	Philosophe* p = philo;
	int qte_penser = 0;
	int qte_manger = 0;

	while(qte_penser < QTE_PENSER_MAX && qte_manger < QTE_MANGER_MAX){

		switch(p->action){

			case MANGER : 
				return_forks();
				p->action = PENSER;
				qte_penser++;
				break;

			default :
				pickup_forks();
				p->action = MANGER;
				qte_manger++;
				break;
		}

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


int main(){

	pthread_t threads [QTE_PHILOSOPHES];
	Philosophe* philosophes [QTE_PHILOSOPHES];

	for(int i = 0 ; i < QTE_PHILOSOPHES ; i++){

		Philosophe* p = initPhilosophe();
		philosophes[i] = p;

		if(pthread_create(&threads[i], NULL, &faire_une_action, philosophes[i]) != 0){
			perror("Une erreur est survenue à l'ouverture de thread.");
			exit(-1);
		}
	}

	for (int i = 0; i < QTE_PHILOSOPHES; i++) {

        if (pthread_join(threads[i], NULL) != 0) {
			perror("Une erreur est survenue à l'attente de terminaison du thread.");
            exit(-2);
        }
    }

}
