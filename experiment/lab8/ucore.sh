I#!/bin/sh
bash ucore-clean.sh
gcc -o xc -O3 -m32 -Ilinux -Iucore/lib ucore/tool/c.c
gcc -o xem -O3 -m32 -Ilinux -Iucore/lib ucore/tool/em.c -lm
g++ -o changeHeader -O3 -m32 ucore/tool/changeHeader.c
cpp-5 -Iucore/lib -Iucore/kern/include -Iucore/kern/libs -Iucore/kern/mm -Iucore/kern/fs -Iucore/kern/driver -Iucore/kern/sync -Iucore/kern/trap -Iucore/kern/process -Iucore/kern/schedule -Iucore/kern/syscall ucore/kern/main.c ucore.c
cpp-5  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/priority.c ucore/user/bin/priority.c
./xc -v -o ucore/user/bin/priority ucore/user/bin/priority.c
./changeHeader ucore/user/bin/priority
./xc -v -o ucore.bin ucore.c 
./xem -f ucore/user/bin/priority ucore.bin
