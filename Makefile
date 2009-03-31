all: topartists lfmfs

lfmfs: main.c
	gcc -g -O0 -c `pkg-config --cflags fuse` -o lfmfs.o $?
	g++ -g -O0 `pkg-config --libs fuse` `curl-config --libs` -lexpat topartists.o lfmfs.o -o lfmfs

topartists: topartists.cpp
	gcc -g -O0 -c -o topartists.o `curl-config --cflags` $?
