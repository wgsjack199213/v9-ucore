#!/bin/sh
gcc -o xc -O3 -m32 -Ilinux -Iucore/lib ucore/tool/c.c
gcc -o xem -O3 -m32 -Ilinux -Iucore/lib ucore/tool/em.c -lm
cpp-4.9 -Iucore/lib ucore/kern/main.c ucore.c
./xc -v -o ucore.bin ucore.c 
./xem ucore.bin
