all:
	cd threadpool && make
	cd c_printf && make
	gcc -Ithreadpool/src -Ic_printf/sources -c -o src/dfc.o src/dfc.c
	gcc -Ithreadpool/src -Isrc -c -o test/add_runable.o test/add_runable.c
	gcc test/add_runable.o src/dfc.o threadpool/src/threadpool.o c_printf/sources/c_printf.o -lpthread -o test/add_runable
	gcc -Ithreadpool/src -Isrc -c -o test/bubbld_runable.o test/bubbld_runable.c
	gcc test/bubbld_runable.o src/dfc.o threadpool/src/threadpool.o c_printf/sources/c_printf.o -lpthread -o test/bubble
	gcc -Ithreadpool/src -Isrc -c -o test/bubbld_bean.o test/bubbld_bean.c
	gcc test/bubbld_bean.o src/dfc.o threadpool/src/threadpool.o c_printf/sources/c_printf.o -lpthread -o test/bubbld_bean

clean:
	cd threadpool && make clean
	cd c_printf && make clean
	rm src/dfc.o
	rm test/add_runable.o
	rm test/add_runable
