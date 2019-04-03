all:
	 gcc -Ithreadpool/src -c -o main.o main.c
	 gcc main.o threadpool/src/threadpool.o -lpthread -o main

clean:
	rm main.o
	rm main
