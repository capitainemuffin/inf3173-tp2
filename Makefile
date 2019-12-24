options=-Wall -pedantic -std=c11 -D_POSIX_SOURCE


.PHONY : clean push

default : tp2 valider 

tp2 : tp2.o
	gcc -o $@ $< $(options) -lpthread 

tp2.o : tp2.c 
	gcc -c -o $@ $< $(options)

valider : valider.o
	gcc -o $@ $< $(options)

valider.o : valider.c
	gcc -c -o $@ $< $(options)

clean : 
	rm -fr *.o
	rm -fr *.out
	rm -fr tp2
	rm -fr valider
	rm -fr resultat.txt

push :
	make clean
	git add .
	git commit -m 'automated commit'
	git push origin master


