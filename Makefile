options=-Wall -pedantic -std=c11 -lpthread

.PHONY : clean push

default : tp1 valider 

tp1 : tp1.o
	gcc -o $@ $< $(options)

tp1.o : tp1.c 
	gcc -c -o $@ $< $(options)

valider : valider.o
	gcc -o $@ $< $(options)

valider.o : valider.c
	gcc -c -o $@ $< $(options)

clean : 
	rm -fr *.o
	rm -fr tp1 
	rm -fr valider

push :
	make clean
	git add .
	git commit -m 'automated commit'
	git push origin master


