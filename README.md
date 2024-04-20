gcc -c main.c
gcc -c gamelib.c
gcc -o game main.o gamelib.o -std=c11 -Wall
