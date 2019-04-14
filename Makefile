all:
	 gcc -Ithreadpool/src -Ic_printf/sources -c -o dfc.o dfc.c
	 gcc -Ithreadpool/src -c -o main.o main.c
	 gcc main.o dfc.o threadpool/src/threadpool.o c_printf/sources/c_printf.o -lpthread -o main

clean:
	rm main.o
	rm main
