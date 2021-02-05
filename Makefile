#Name : Suyashi Singhal 
#Roll no: 2019478
compile  : compile_blocking compile_nonBlocking

compile_blocking:	blocking_2019478.c
	gcc -pthread blocking_2019478.c -o blocking

compile_nonBlocking: non_blocking_2019478.c
	gcc -pthread non_blocking_2019478.c -o non_blocking

run_blocking : blocking
	./blocking -pthread

run_nonBlocking: non_blocking
	./non_blocking -pthread

clean:
	rm blocking
	rm non_blocking