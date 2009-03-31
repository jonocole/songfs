all: topartists lfmfs

lfmfs: main.c
	gcc -c `pkg-config --cflags fuse` -o lfmfs.o $?
	g++ `pkg-config --libs fuse` `curl-config --libs` -lexpat topartists.o lfmfs.o -o lfmfs

topartists: topartists.cpp
	g++ -c -o topartists.o `curl-config --cflags` $?
