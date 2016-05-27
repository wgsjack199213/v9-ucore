#!/bin/sh
bash ucore-clean.sh
gcc -o xc -O3 -m32 -Ilinux -Iucore/lib ucore/tool/c.c
gcc -o xem -O3 -m32 -Ilinux -Iucore/lib ucore/tool/em.c -lm
cpp-5 -Iucore/lib -Iucore/kern/include -Iucore/kern/libs -Iucore/kern/mm -Iucore/kern/fs -Iucore/kern/driver -Iucore/kern/sync -Iucore/kern/trap -Iucore/kern/process -Iucore/kern/schedule -Iucore/kern/syscall ucore/kern/main.c ucore.c
./xc -v -o ucore.bin ucore.c 
./xem ucore.bin
