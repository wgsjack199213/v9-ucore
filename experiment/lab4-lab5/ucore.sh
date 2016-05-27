#!/bin/sh
bash ucore-clean.sh
gcc -o xc -O3 -m32 -Ilinux -Iucore/lib ucore/tool/c.c
gcc -o xem -O3 -m32 -Ilinux -Iucore/lib ucore/tool/em.c -lm
g++ -o changeHeader -O3 -m32 ucore/tool/changeHeader.c
cpp-5 -Iucore/lib -Iucore/kern/include -Iucore/kern/libs -Iucore/kern/mm -Iucore/kern/fs -Iucore/kern/driver -Iucore/kern/sync -Iucore/kern/trap -Iucore/kern/process -Iucore/kern/schedule -Iucore/kern/syscall ucore/kern/main.c ucore.c
cpp-5  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/exit.c ucore/user/bin/exit.c
./xc -v -o ucore/user/bin/exit ucore/user/bin/exit.c
./changeHeader ucore/user/bin/exit
./xc -v -o ucore.bin ucore.c 
./xem -f ucore/user/bin/exit ucore.bin
