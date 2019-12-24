# INF3173 - TP2 

## Compilation 
- Le programme `tp2` se compile ainsi : 
```
gcc -o tp2 tp2.c -Wall -pedantic -std=c11 -D_POSIX_SOURCE -lpthread 
```
- Le programme `valider` se compile ainsi : 
```
gcc -o valider valider.c -Wall -pedantic -std=c11 -D_POSIX_SOURCE
```

Un fichier `Makefile` est aussi inclus dans le travail. 
Il suffit d'entrer la commande `make` dans le terminal pour tout compiler. 
Les executables `tp2` et `valider` seront ainsi générés. 

## Fonctionnement

Il faut commencer par exécuter le programme `tp2`.
Celui-ci va générer le fichier `resultat.txt`.

On peut alors utiliser le programme `valider` et faire les tests nécessaires.

Le programme est fonctionnel sous Mac et Linux, il a été testé sur le serveur java.labunix de l'UQAM.