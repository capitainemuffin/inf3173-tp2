/**
* TODO : 
* utiliser un lock mutex avant de prendre 2 baguettes (décrémenter) puis manger 5 secondes 
* s'il ne reste pas assez de baguette : penser 
* 
**/


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
#define QTE_MANGER 5
#define QTE_PENSER 5

/**
* Le temps maximum et minimum qu'un philosophe peut prendre pour une action
**/
#define TEMPS_MAX 3
#define TEMPS_MIN 1

int nbr_baguettes_disponibles = QTE_BAGUETTES;
pthread_mutex_t mutex; 

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
	p->qte_baguettes_en_main = 0;
	p->action = MANGER;
	p->qte_penser = 0;
	p->qte_manger = 0;
	return p;

}

/**
* Destructeur d'un philosophe
* Libère la mémoire allouée à la structure <Philosophe> p
**/
void freePhilosophe(Philosophe* p){

	free(p);

}

void pickup_forks(Philosophe* philo){

	Philosophe* p = philo;

	if(nbr_baguettes_disponibles > 0){
		nbr_baguettes_disponibles--;
		p->qte_baguettes_en_main++;
	}

	if(nbr_baguettes_disponibles > 0){
		nbr_baguettes_disponibles--;
		p->qte_baguettes_en_main++;
	}

	if(p->qte_baguettes_en_main == 2){
		p->action = MANGER;
		p->qte_manger++;
	}

}


void return_forks(Philosophe* philo){

	Philosophe* p = philo;

	p->qte_baguettes_en_main = 0;
	nbr_baguettes_disponibles += 2;
	p->action = PENSER;
	p->qte_penser++;

}

void* faire_une_action(void* philo){

	Philosophe* p = philo;

	while(1){

		if(p->qte_penser == 5 && p->qte_manger == 5) break;
		if(p->qte_manger < 5) pickup_forks(p);

		// Temps pris pour faire l'action
		int temps = (rand() % (TEMPS_MAX + 1 - TEMPS_MIN)) + TEMPS_MIN;
  		#ifdef _WIN32
  		Sleep(temps * 1000);
  		#else
  		sleep(temps);
  		#endif

  		return_forks(p);
	}


}


int main(){

	pthread_t threads [QTE_PHILOSOPHES];
	Philosophe* philosophes [QTE_PHILOSOPHES];

	for(int i = 0 ; i < QTE_PHILOSOPHES ; i++){

		Philosophe* p = initPhilosophe();
		philosophes[i] = p;
		pthread_create(&threads[i], NULL, &faire_une_action, philosophes[i]);

	}


}


