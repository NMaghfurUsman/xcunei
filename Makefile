default: main

main: main.o uitk.o
	gcc -g -Wall -lxcb -lX11 -lxcb-xtest -lxcb-icccm -lm -lX11-xcb -o xcunei main.o uitk.o

main.o: main.c
	gcc -g -Wall -c main.c

uitk.o: uitk.h uitk_x11_impl.c
	gcc -g -Wall -c uitk_x11_impl.c -o uitk.o

clean:
	rm *.o; rm xcunei
