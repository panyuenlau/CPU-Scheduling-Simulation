#!/bin/sh

# test _1 compile (gcc; g++; javac; python -m py_compile project1.py)

gcc -Wall project.c -lm -D DISPLAY_MAX_T=1000

# test _2
./a.out 2 0.01 256 1 4 0.5 128 > output02_test.txt
mv simout.txt simout02.txt

# test _3
./a.out 2 0.01 256 2 4 0.5 128 > output03_test.txt
mv simout.txt simout03.txt

# test _4
./a.out 2 0.01 256 16 4 0.5 128 > output04_test.txt
mv simout.txt simout04.txt

# test _5
./a.out 64 0.001 4096 8 4 0.5 2048 > output05_test.txt
mv simout.txt simout05.txt
