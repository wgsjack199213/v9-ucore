#!/bin/sh
bash ucore-clean.sh
gcc -o xc -O3 -m32 -Ilinux -Iucore/lib ucore/tool/c.c
gcc -o xem -O3 -m32 -Ilinux -Iucore/lib ucore/tool/em.c -lm
gcc -o xmkfs -O3 -m32 -Ilinux -Iucore/lib ucore/tool/mkfs.c
cpp-4.8 -Iucore/lib -Iucore/kern/include -Iucore/kern/libs -Iucore/kern/mm -Iucore/kern/fs -Iucore/kern/driver -Iucore/kern/sync -Iucore/kern/trap -Iucore/kern/process -Iucore/kern/schedule -Iucore/kern/syscall ucore/kern/main.c ucore.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/sh.c ucore/user/bin/sh.c
./xc -v -o ucore/user/bin/sh ucore/user/bin/sh.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/priority.c ucore/user/bin/priority.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/hello.c ucore/user/bin/hello.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/forktree.c ucore/user/bin/forktree.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/faultreadkernel.c ucore/user/bin/faultreadkernel.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/faultread.c ucore/user/bin/faultread.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/exit.c ucore/user/bin/exit.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/divzero.c ucore/user/bin/divzero.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/badarg.c ucore/user/bin/badarg.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/pgdir.c ucore/user/bin/pgdir.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/matrix.c ucore/user/bin/matrix.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/spin.c ucore/user/bin/spin.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/waitkill.c ucore/user/bin/waitkill.c
cpp-4.8  -Iucore/user/libs -Iucore/lib -Iucore/kern/include ucore/user/yield.c ucore/user/bin/yield.c

./xc -v -o ucore/user/bin/priority ucore/user/bin/priority.c
./xc -v -o ucore/user/bin/hello ucore/user/bin/hello.c
./xc -v -o ucore/user/bin/forktree ucore/user/bin/forktree.c
./xc -v -o ucore/user/bin/faultreadkernel ucore/user/bin/faultreadkernel.c
./xc -v -o ucore/user/bin/faultread ucore/user/bin/faultread.c
./xc -v -o ucore/user/bin/exit ucore/user/bin/exit.c
./xc -v -o ucore/user/bin/divzero ucore/user/bin/divzero.c
./xc -v -o ucore/user/bin/badarg ucore/user/bin/badarg.c
./xc -v -o ucore/user/bin/pgdir ucore/user/bin/pgdir.c
./xc -v -o ucore/user/bin/matrix ucore/user/bin/matrix.c
./xc -v -o ucore/user/bin/spin ucore/user/bin/spin.c
./xc -v -o ucore/user/bin/waitkill ucore/user/bin/waitkill.c
./xc -v -o ucore/user/bin/yield ucore/user/bin/yield.c

./xmkfs user.img ucore/user/bin
./xc -v -o ucore.bin ucore.c
./xem -f user.img ucore.bin
