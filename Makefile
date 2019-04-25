all:
	cd threadpool && make
	cd c_printf && make
	gcc -c -o c_printf/sources/c_printf.o c_printf/sources/c_printf.c
	gcc -Ithreadpool/src -Ic_printf/sources -c -o src/dfc.o src/dfc.c
	gcc -Ithreadpool/src -Isrc -c -o test/add_runable.o test/add_runable.c
	gcc test/add_runable.o src/dfc.o threadpool/src/threadpool.o c_printf/sources/c_printf.o -lpthread -o test/add_runable
	gcc -Ithreadpool/src -Isrc -c -o test/bubbld_bean.o test/bubbld_bean.c
	gcc test/bubbld_bean.o src/dfc.o threadpool/src/threadpool.o c_printf/sources/c_printf.o -lpthread -o test/bubbld_bean
	gcc -Ithreadpool/src -Isrc -c -o test/bubbld_more.o test/bubbld_more.c
	gcc test/bubbld_more.o src/dfc.o threadpool/src/threadpool.o c_printf/sources/c_printf.o -lpthread -o test/bubbld_more
	gcc -Ithreadpool/src -Isrc -c -o test/matrix_runable.o test/matrix_runable.c
	gcc test/matrix_runable.o src/dfc.o threadpool/src/threadpool.o c_printf/sources/c_printf.o -lpthread -o test/matrix_runable
	gcc -Ithreadpool/src -Isrc -c -o test/complex_test_runable.o test/complex_test_runable.c
	gcc test/complex_test_runable.o src/dfc.o threadpool/src/threadpool.o c_printf/sources/c_printf.o -lpthread -o test/complex_test_runable

clean:
	cd threadpool && make clean
	cd c_printf && make clean
	rm src/dfc.o
	rm test/add_runable.o
	rm test/add_runable
	rm test/bubbld_bean.o
	rm test/bubbld_bean
	rm test/bubbld_more.o
	rm test/bubbld_more
	rm test/matrix_runable.o
	rm test/matrix_runable
