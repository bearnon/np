all:
	g++ -o client client.cpp
	g++ -o server server.cpp

test: all
	gcc -o test test.c
	stdbuf -oL ./test

clean:
	rm -f server client test
