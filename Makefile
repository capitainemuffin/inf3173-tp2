options=-Wall -pedantic -std=c11 

.PHONY : clean 

default : tp1

tp1 : tp1.o
	gcc -o $@ $< $(options)

tp1.o : tp1.c 
	gcc -c -o $@ $< $(options)

clean : 
	rm -fr *.o 

push :
	git add .
	git commit -m 'automated commit'
	git push origin master
