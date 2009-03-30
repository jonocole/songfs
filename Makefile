all: topartists

lfmfs:
	gcc -o lfmfs `pkg-config --cflags --libs fuse` main.c

topartists:
	g++ -o topartists -I /opt/local/include `curl-config --cflags --libs` -L/opt/local/lib/ -lexpat topartists.cpp
