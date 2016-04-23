#!/bin/sh
#gcc -o xc -O3 -m32 -Ilinux ucore/tool/c.c
#gcc -o xem -O3 -m32 -Ilinux ucore/tool/em.c -lm
cpp-5 -Iucore/lib ucore/kern/main.c ucore.c
./xc -v -o ucore.bin ucore.c 
#./xem ucore.bin
