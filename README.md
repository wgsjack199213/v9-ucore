#ucore在v9-CPU上的移植
**韩旭 徐磊**

## 概述
将ucore移植到v9-CPU上是我们组课程设计的核心内容。根据我们的设想，我们将完成ucore代码（包含了lab1~lab8）在v9-CPU上的移植工作，我们进一步的希望能实现更多的操作系统内存管理、进程管理的策略算法，并能够提供更好的实验是指导书。

由于对于代码移植部分的困难估计不足，因此目前只完成了实验的主体部分，即代码移植。目前我们已经将ucore移植到v9-CPU上，相关代码可以较为稳定的运行。我们将8个实验进行了一定的合并，合并之后的包括了6个实验。目前移植的实验和ucore保持功能一致，但在某些硬件相关的部分没有完全按照ucore的抽象层次实现，这样可以保持程序的简单。

## 相关工作介绍

**[swieros](https://github.com/rswier/swieros)** 是v9-CPU的原型，作者设计了一个兼具RISC和SISC特性的指令集，并为指令集实现了一个简化的C编译器以及相应的模拟器。作者在这套框架上移植了xv6教学操作系统，但是受限于编译器和模拟器，xv6被较多的修改，而且在实现上也有一些不太合理的地方。

**[ucore](https://github.com/chyyuu/ucore_os_lab)** 是清华大学操作系统课程采用的教学操作系统，这套操作系统具有较为完善的抽象层次和代码结构，操作系统被拆分成了8个lab，每个lab都在前一个lab上进行了一些修改和加入了新的功能。比较适合增量开发，但是这样的开发也可能造成一些麻烦。

**[v9.js](https://github.com/JianxinMa/v9.js)** 是今年课程设计另一个小组的工作，它把v9的编译器和调试器都移植到javascript上，这样就可以有一个在浏览器上的运行环境。我们希望我们移植之后的操作系统可以在他们提供的平台上运行。

**[alex-machine](https://github.com/paulzfm/alex-machine)** 参考v9定义的新指令集和模拟器。

**[alex-llvm](https://github.com/a1exwang/llvm)** 采用LLVM作为C语言的编译器，这样就可以使用标准的C进行操作系统的编程而不必考虑编译器带来的各种局限。因为alex与v9非常类似，将来可能可以把v9-ucore通过比较少的修改移植到alex-machine上。

## 实现方案

我们采取的方案核心思路是：

- 保持代码结构ucore内核基本一致
- 和硬件相关功能参考xv6的实现，采用较为简单的方法实现
- 合并相关lab，保留一部分中间结果，但避免多次lab之间的迁移的麻烦

## 主要难点

**非标准编译器带来的麻烦**

- 编译器不支持const，使用const编译器不仅不会报错，而且会产生难以预期的结果，因此这一问题一定要加以注意。
- 编译器不支持宏定义，只支持一层include，这会带来很多工程上的问题，我们使用GCC的代码预处理器解决了这个问题。这样所有宏定义都有了支持，而且可以比较好的处理复杂的引用关系。经过预处理后的程序是一个单独的文件，更加简单，更有可能复合已有编译器的要求。
- 编译器对于函数的声明和定义有一定的限制。在标准C中，函数可以在任意位置声明任意多次，但是在现有的编译器上，函数声明只能出现在定义之前，且只能有一次，这个问题编译器会报错，因此稍加修改就可以避免。
- 编译器不支持链接，所以需要讲所有的函数都写在头文件中，这样很容易出现多个头文件循环依赖的情况，这时候就需要对文件结构进行一些修改，把函数的声明单独抽象成一个头文件。
- 编译器支持函数指针，但是写法比较特殊，我们到实验的后期才发现了这个特性，所以我们使用了一些特殊的方法手动修改调用栈，实现了相关功能。

**硬件相关的代码**

- 操作系统底层有很多与硬件密切相关的代码，这部分代码随着平台的切换就需要完全的进行修改。实验的核心也就是修改这部分代码。
- X86作为SISC，硬件能提供很多复杂的功能，我们需要使用多条指令对单指令进行替换，这时候就需要考虑原子操作的问题，我们使用开关中断处理这一问题。
- 寄存器的问题，v9-CPU上只有3个寄存器，在系统调用时可能出现传参数不够用的情况，我们使用传指针解决这个问题，这样会造成一些系统调用的接口有所变化，以及一些代码不一致产生的问题。
- 某些硬件的接口完全是不同的，例如X86上磁盘采用寄存器控制，而v9-CPU上，磁盘是一块被映射的内存，两者采用的文件系统也是完全不同的，这是需要将xv6中的相关驱动移植到ucore里。

**ucore中复杂的抽象层次**

- 一个显著的例子是文件系统，ucore中的文件系统使用Linux内核中移植的，非常复杂，很难修改支持硬件。我们直接使用了xv6中的驱动，在系统调用层面把文件系统和内核的其它部分做了接口的转换。

## 开发过程和运行流程解析

###Lab1-Lab2

####*	启动脚本描述

```
#!/bin/sh
bash ucore-clean.sh		#清空已有的运行部件
gcc -o xc -O3 -m32 -Ilinux -Iucore/lib ucore/tool/c.c	#编译v9的C编译器
gcc -o xem -O3 -m32 -Ilinux -Iucore/lib ucore/tool/em.c -lm	#编译v9的C模拟器
cpp -Iucore/lib -Iucore/kern/include -Iucore/kern/libs -Iucore/kern/mm -Iucore/kern/fs -Iucore/kern/driver -Iucore/kern/sync -Iucore/kern/trap -Iucore/kern/process -Iucore/kern/schedule -Iucore/kern/syscall ucore/kern/main.c ucore.c	#预编译整个v9-ucore的程序，将库文件合成一个程序进行编译
./xc -v -o ucore.bin ucore.c 	#编译v9-ucore
./xem ucore.bin		#在模拟器上运行v9-ucore
```

####*	程序结构解析


```
	u.h					--->	v9相关的一些定义
	sync.h				--->	开关中断并保存状态的操作定义
	atomic.h			--->	置位运算的原子操作定义
	call.h				--->	进行函数指针调用的函数定义
	defs.h				--->	ucore的相关定义
	error.h				--->	ucore的错误定义
	list.h				--->	指针以及其相关方法的定义
	io.h				--->	底层读写的定义
	printfmt.h			--->	printf的定义
	string.h			--->	string以及内存复制拷贝的底层操作定义
	
	mm
	|--default_pmm.h	--->	物理页分配算法实现
	|--memlayout.h		--->	物理空间的构成定义
	|--mmu.h			--->	物理地址操作的定义
	|--pmm.h			--->	物理地址相关操作的实现
	
	trap
	|--trap.h			--->	trap的相关实现
	
	main.c				--->	操作系统程序入口
	
```	

####*	必要细节梳理（main.c）

```
static char kstack[4096]; 		// 临时的内核栈

void kern_init() {
  pmm_init();                   // 物理地址相关操作初始化
  idt_init();                   // 中断的初始化
  stmr(128*1024*1000);          // 打开时钟中断开关
  asm (STI);                    // 打开中断开关
  while (1) {
    // do nothing
  }
}

main() {
  int *ksp;
  static int bss;
  endbss = &bss;				//在分配内存的时候，我们要清楚的知道text段，data段和bss段的大小，bss段管理的是未初始化的静态变量，在此处声明一个静态变量，通过获取地址来得到bss段的边界
  ksp = ((uint)kstack + sizeof(kstack) - 8) & -8;
  asm (LL, 4);					//a<-*(sp+4)
  asm (SSP);					//sp<-a，在bss段中开取一段来用作系统的内核栈，如果没有这个操作，此时分配的将是物理内存的顶部，实际上物理内存的bss段以上的部分应进出页划分并通过分配算法来分配，如果使用这部分的空间作为系统内核栈将复杂许多，换句话说操作系统text、data、bss段占用的空间是不计入操作系统能够分配的空间中去的，这部分代码也不会在任何情况下从内存中被换入换出。
  kern_init();
  asm (HALT);
}

```

####*	必要细节梳理（trap.h）

```
struct pushregs {
    int sp, pad1;
    double g;
    double f;
    int c,  pad2;
    int b,  pad3;
    int a,  pad4;
};

struct trapframe {
    struct pushregs tf_regs;
    int fc, pad5;
    int pc, pad6;
};

//trapframe的定义，其中pad之类的是用以填充空间的，因为v9压栈是64位对齐的压栈，但寄存器都是32位的，所以多余的位也要压入填充量用以对齐，浮点寄存器由于就是64位的所以可以不用填充量。struct的空间排布就是内部成员的排布，换言之上述的定义与下述是等价的
//	struct trapframe {
//    	int sp, pad1;
//    	double g;
//    	double f;
//    	int c,  pad2;
//    	int b,  pad3;
//    	int a,  pad4;
//  	int fc, pad5;
//	    int pc, pad6;
//	};

enum {    	// processor fault codes
    FMEM,   // bad physical address
    FTIMER, // timer interrupt
    FKEYBD, // keyboard interrupt
    FPRIV,  // privileged instruction
    FINST,  // illegal instruction
    FSYS,   // software trap
    FARITH, // arithmetic trap
    FIPAGE, // page fault on opcode fetch
    FWPAGE, // page fault on write
    FRPAGE, // page fault on read
    USER=16 // user mode exception
};

//trap的类型，即下文fc(fault code)的取值有上述几种可能性，内核和用户的区分就是通过USER来进行的，例如fc为FTIMER表示内核栈的时钟中断，fc为USER+FTIMER表示用户栈的时钟中断
```

```
void trap_dispatch(struct trapframe *tf) {
	...
	...
	...
}

void trap(uint *sp, double g, double f, int c, int b, int a, int fc, uint *pc) {
  trap_dispatch(&sp);			//将零散压入的寄存器量通过一个trapframe的指针来代替，从而方便调用，跳入trap_dispatch来真正进行中断操作
}

void alltraps()
{
  asm(PSHA);					//中断发生时会将当前的寄存器状况和中断类型、指令地址等压入内核栈中，用以在中断处理完成后能够恢复出现中断的现场寄存器环境从而继续进行下去
  asm(PSHB);					//这部分汇编则是在压入a、b、c三个通用寄存器以及f、g浮点寄存器和当前sp的地址，如果在用户态则压入的是用户态的sp地址，在内核态则压入的是内核态的sp地址
  asm(PSHC);					//fc中断类型和pc指令地址则是硬件压入栈的，不需要要软件插手
  asm(PSHF);
  asm(PSHG);
  asm(LUSP); asm(PSHA);
  trap();						//trapframe全部压入后跳转到trap进行中断处理操作
  asm(POPA); asm(SUSP);			//将trapframe中的元素逐一弹出达到恢复现场的目的
  asm(POPG);
  asm(POPF);
  asm(POPC);
  asm(POPB);
  asm(POPA);
  asm(RTI);						//将pc弹出返回中断发生处继续工作
}

void idt_init() {
  ivec(alltraps);				//将alltraps函数地址设置为中断发生时的跳转地址，换句话说，中断发生时会跳到alltraps进行中断操作
}
```

####*	必要细节梳理（memlayout.h）

```
/* *
 * Virtual memory map:                                          Permissions
 *                                                              kernel/user
 *
 *     4G ------------------> +---------------------------------+
 *                            |                                 |
 *                            |         Empty Memory (*)        |
 *                            |                                 |
 *                            +---------------------------------+ 0xFB000000
 *                            |   Cur. Page Table (Kern, RW)    | RW/-- PTSIZE
 *     VPT -----------------> +---------------------------------+ 0xFAC00000
 *                            |        Invalid Memory (*)       | --/--
 *     KERNTOP -------------> +---------------------------------+ 0xF8000000
 *                            |                                 |
 *                            |    Remapped Physical Memory     | RW/-- KMEMSIZE
 *                            |                                 |
 *     KERNBASE ------------> +---------------------------------+ 0xC0000000
 *                            |                                 |
 *                            |                                 |
 *                            |                                 |
 *                            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * (*) Note: The kernel ensures that "Invalid Memory" is *never* mapped.
 *     "Empty Memory" is normally unmapped, but user programs may map pages
 *     there if desired.
 *
 * */
/* All physical memory mapped at this address */
#define KERNBASE            0xC0000000
#define KMEMSIZE            0x38000000                  // the maximum amount of physical memory
#define KERNTOP             (KERNBASE + KMEMSIZE)
/* *
 * Virtual page table. Entry PDX[VPT] in the PD (Page Directory) contains
 * a pointer to the page directory itself, thereby turning the PD into a page
 * table, which maps all the PTEs (Page Table Entry) containing the page mappings
 * for the entire virtual address space into that 4 Meg region starting at VPT.
 * */
#define VPT                 0xFAC00000
#define KSTACKPAGE          2                           // # of pages in kernel stack
#define KSTACKSIZE          (KSTACKPAGE * PGSIZE)       // sizeof kernel stack
```

*	这部分是内存空间的排布，32位能够分配4G的空间，其中内核程序都应放置到0xC0000000以上部分，页表放置在0xFAC00000开始的连续空间内，事实上在页机制未启动时所有的段都是分布在0x00000000上的，且物理地址等于逻辑地址，在pmm_init中要做的就是初始化页表使得操作系统的各段的虚拟地址变为0xC0000000，或者说是0xC0000000指向物理地址0x00000000。


####*	必要细节梳理（pmm.h）

```
void
page_init(void) {
  uintptr_t freemem;
  uint32_t maxpa, begin, end;
  int i;
  maxpa = msiz();							//获取物理内存大小
  if (maxpa > KMEMSIZE) {
    maxpa = KMEMSIZE;
  }
  npage = maxpa / PGSIZE;					//所有的物理内存一共能划分出的页数
  pages = (struct Page *)ROUNDUP((uint)(endbss) + PGSIZE + 3, PGSIZE);	//只有bss段以外的部分才是可以动态分配的空间，换句话说操作系统数据和代码的段是不算进之后能够使用的空间内的
  for (i = 0; i < npage; i++) {
    SetPageReserved(pages + i);				//初始化所有的页的标志位
  }
  freemem = ((uintptr_t)pages + sizeof(struct Page) * npage);
  begin = freemem;
  end = maxpa;
  if (begin < end) {						//每个可以分配的页都将建立一个页头结构来管理，即struct Page，从pages之后连续的若干个struct Page将所有能分配的物理页串联起来用以分配算法，这里就是初始化部分
    begin = ROUNDUP(begin, PGSIZE);
    end = ROUNDDOWN(end, PGSIZE);
    if (begin < end)
      init_memmap(pa2page(begin), (end - begin) / PGSIZE);
  }
}
```

```
void check_and_return(void) {
  list_entry_t *head = &free_area.free_list;
  boot_pgdir = (uint)(boot_pgdir) + KERNBASE;	//在启动页表机制之后，页表的虚拟地址不再是0x00000000向上的部分而是0xC0000000向上的部分，所以加上KERNBASE，即0xC0000000
  boot_pgdir[0] = 0;							//在完成过渡之后，所有的指令地址都处于0xC0000000向上，此时0x00000000和0xC0000000映射到相同的物理内存已经不需要了
  load_default_pmm_manager();					//在页机制算法初始化时函数指针也是0x00000000向上的部分，重新植入函数指针将函数指针指向到0xC0000000向上的部分
  pmm_manager = &default_pmm_manager;
  pages = (uint)(pages) + KERNBASE;
  while (1) {									//在页表的管理头里的指针都是0x00000000为基址时的状态，在页机制启动后，其中的指针值并不会随之改变，要遍历一次进行更新
    head->prev = (uint)(head->prev) + KERNBASE;
    head->next = (uint)(head->next) + KERNBASE;
    if (head->next == &free_area.free_list) break;
    head = list_next(head);
  }
  check_boot_pgdir();							//均进行完成后页机制已经启动完成，这里是检测机制，与页机制本身无关
  print_pgdir();
}

```

```
uint getsp() {
  asm (LEA, 8);							//获取栈顶的函数，由于在调用这个函数本身要多压入一个返回地址，所以汇编获得sp的值之后要加上8（v9压栈64位对齐）才是调用函数的程序段中的栈地址
}

void
pmm_init(void) {
  int *ksp;
  page_enable = 0;	
  init_pmm_manager();					//页机制算法的初始化
  page_init();							//页表头的初始化
  check_alloc_page();					
  boot_pgdir = boot_alloc_page();
  memset(boot_pgdir, 0, PGSIZE);		
  boot_cr3 = boot_pgdir;				//boot_cr3是一级页表的物理地址
  check_pgdir();
  boot_pgdir[PDX(VPT)] = (uint)(boot_pgdir) | PTE_P | PTE_W;
  boot_map_segment(boot_pgdir, KERNBASE, KMEMSIZE, 0, PTE_W);	//页表的初始化设置将0xC0000000到0xC0000000+KMEMSIZE映射到0x00000000到0x00000000+KMEMSIZE
  boot_pgdir[0] = boot_pgdir[PDX(KERNBASE)];	//页表启动后部分指令地址和变量地址还在0x00000000上而不是0xC0000000，所以此时需要0xC0000000和0x00000000指向同样的物理地址
  page_enable = 1;						//页机制启动后的标志
  pdir(boot_cr3);						//设置一级页表的物理地址
  spage(1);								//打开页机制
  ksp = KERNBASE + getsp();				//页机制启动后sp应变为0xC0000000+sp
  asm (LL, 4);							//a<-*(ksp+4)，即a<-ksp
  asm (SSP);							//sp<-a，即sp<-0xC0000000+sp
  *(uint *)(getsp()+8) += KERNBASE;		//sp上的函数kern_init的返回地址+0xC0000000
  *(uint *)(getsp()+16) += KERNBASE;	//sp上的函数main的返回地址+0xC0000000
  ksp = (uint)(check_and_return) + KERNBASE;	//此时的指令地址还是处于0x00000000上的部分，此时要强行的将之后的执行部分+0xC0000000并跳转进行执行，之后的部分就是check_and_return
  asm (LL, 4);
  asm (JSRA);
}
```

####*	ucore实现上的技巧说明

```
/* Return the offset of 'member' relative to the beginning of a struct type */
#define offsetof(type, member)                                      \
  ((size_t)(&((type *)0)->member))

/* *
 * to_struct - get the struct from a ptr
 * @ptr:    a struct pointer of member
 * @type:   the type of the struct this is embedded in
 * @member: the name of the member within the struct
 * */
#define to_struct(ptr, type, member)                               \
  ((type *)((char *)(ptr) - offsetof(type, member)))

```

*	这是ucore中很常用的定义，offsetof(type, member)可以计算struct中的某一个member在这个struct中的偏移量，to_struct则根据struct中某一个member的地址得到struct的地址，这个巧妙的实现使得很多实现变的很简单。

```
ppn_t
page2ppn(struct Page *page) {
  return page - pages;
}

uintptr_t
page2pa(struct Page *page) {
  return page2ppn(page) << PGSHIFT;
}
```

*	物理页的申请获得的都是页头即struct Page，将struct Page换为对应的物理地址则通过上述函数实现，pages是页头的起始位置，指针相减可以得到当前的页头是第几个页的页头，然后偏移12位即可，即第0页对应0x0000，第1页对应0x1000，第2页对应0x2000。

*	页的相关信息科参考<a src="https://objectkuan.gitbooks.io/ucore-docs/content/lab2/lab2_3_3_3_phymem_pagelevel.html">实验参考书</a>相关章节

###Lab3

####*	磁盘构建

*	v9底层是没有成熟的磁盘接口的，所以在这里我们在内存中开出了一块空间来构建了一块虚拟磁盘。

```
static char disk[1 * 1024 * 8 * 512];	//在bss段中开出空间构建一块虚拟磁盘作为交换分区

void
ide_init(void) {
  memset(disk, sizeof(disk), 0);		//磁盘初始化即是将bss段中的虚拟磁盘区域清空
}

size_t
ide_device_size(unsigned short ideno) {
  return 1 * 1024 * 8;
}

int										//从磁盘中读则是直接从虚拟磁盘中将内容拷贝进目标地址dst中
ide_read_secs(unsigned short ideno, uint32_t secno, void *dst, size_t nsecs) {
  int ret = 0;
  int i;
  secno = secno * SECTSIZE;
  for (; nsecs > 0; nsecs--, dst += SECTSIZE, secno += SECTSIZE) {
    for (i = 0; i < SECTSIZE; i++) {
      *((char *)(dst) + i) = disk[secno + i];
    }
  }
  return ret;
}

int										//往磁盘中写则是直接从来源地址src中向虚拟磁盘中拷贝内容
ide_write_secs(unsigned short ideno, uint32_t secno, uint32_t *src, size_t nsecs) {
  int ret = 0;
  int i;
  secno = secno * SECTSIZE;
  for (; nsecs > 0; nsecs--, src += SECTSIZE, secno += SECTSIZE) {
    for (i = 0; i < SECTSIZE; i++) {
      disk[secno + i] = *((char *)(src) + i);
    }
  }
  return ret;
}

```

####*	页缺失

```
int
do_pgfault(struct mm_struct *mm, uint32_t error_code, uintptr_t addr) {
  int ret = -E_INVAL;
  struct vma_struct *vma = find_vma(mm, addr);
  struct Page * page;
  pte_t *ptep;
  uint32_t perm;
  pgfault_num++;
  if (vma == NULL || vma->vm_start > addr) {
    printf("not valid addr %x, and  can not find it in vma\n", addr);
    return ret;
  }
  perm = PTE_U;
  if (vma->vm_flags & VM_WRITE) {
    perm |= PTE_W;
  }
  addr = ROUNDDOWN(addr, PGSIZE);
  //由于页机制的最小单位是一页，4KB，所以地址的最后十二位不影响实际寻址，所以我们获得异常地址所在页的起始地址进行修正操作
  ret = -E_NO_MEM;
  ptep = NULL;
  if ((ptep = get_pte(mm->pgdir, addr, 1)) == NULL) {
    printf("get_pte in do_pgfault failed\n");
    return ret;
  }		
  //获取到addr的二级页表
  if (*ptep == 0) {
	//不存在二级页表则构建一个出来并抛出异常
    if (pgdir_alloc_page(mm->pgdir, addr, perm) == NULL) {
      printf("pgdir_alloc_page in do_pgfault failed\n");
      return ret;
    }
  }
  else {
    if(swap_init_ok) {
      page=NULL;		
      //如果二级页表本身已经存在，则将内存中该物理页中的数据复制到对应的交换分区的磁盘中，并将addr映射到内存相应的物理页上
      if ((ret = swap_in(mm, addr, &page)) != 0) {
	printf("swap_in in do_pgfault failed\n");
	return ret;
      }
      spage(1);			
      //在完成映射后要同步清空TLB，由于v9没有TLB指令，在spage打开或者关闭页机制时处理器会完成一次清空TLB操作，所以TLB的清空通过再次打开页机制开关实现
      page_insert(mm->pgdir, page, addr, perm);
      swap_map_swappable(mm, addr, page, 1);
      page->pra_vaddr = addr;
      //构建页表中的addr到物理页的映射从而将页缺失的问题解决
    }
    else {
      printf("no swap_init_ok but ptep is %x, failed\n",*ptep);
      return ret;
    }
  }
  ret = 0;
  return ret;
}

int pgfault_handler(struct trapframe *tf) {
    print_pgfault(tf);
    if (check_mm_struct != NULL) {
        return do_pgfault(check_mm_struct, tf->fc, lvadr());
    }	//在trap中获得缺失地址，通过lvadr获得改地址并跳转到vmm中的响应处理函数解决问题
    panic("unhandled page fault.\n");
}
```

###Lab4-Lab5

Lab4-Lab5分别启动了内核线程和用户进程。进程管理包含进程控制块(pstruct.h)和进程控制(proc.h)及进度调度算法(sched.h)。(1) 进程控制块包括进程的基本信息，用于操作系统管理进程。(2) 进程控制包含了进程的fork, execve, exit等方法，相应的会涉及到一些数据结构和对进程控制块的修改，内存分配、中断等问题。(3)调度算法是进程切换的策略。

其中(1)和(3)与硬件基本无关，可以使用X86的ucore的代码。对于(2)中和进程的内存相关的代码，和硬件密切相关。下面列出了重点修改的代码片段。

切换内核栈，由于汇编指令的差异，需要修改变换内核栈的代码。

```
void switch_to(struct context *oldc, struct context *newc) // switch stacks
{
  oldc->pc = *(uint*)(getsp());
  oldc->sp = getsp() + 8;
  oldc->b = getb();
  oldc->c = getc();

  setb(newc->b);
  setc(newc->c);
  *(uint*)((uint)(newc->sp) - 8) = newc->pc;
  seta((uint)(newc->sp) - 8);
  asm (SSP);
  asm (LEV);
}
```

复制进程，在这一部分，造成差异的是寄存器的不同。

```
// copy_thread - setup the trapframe on the  process's kernel stack top and
//             - setup the kernel entry point and stack of process
static void
copy_thread(struct proc_struct *proc, uintptr_t esp, struct trapframe *tf) {

  proc->tf = (struct trapframe *)(proc->kstack + KSTACKSIZE) - 1;
  memcpy(proc->tf, tf, sizeof(struct trapframe));
  proc->tf->tf_regs.a = 0;
  proc->tf->tf_regs.sp = esp;
  proc->tf->fc = (proc->tf->fc);  // | 16;

  proc->context.pc = (uintptr_t)(forkret);
  proc->context.sp = (uintptr_t)(proc->tf);
}
```

代码加载load_icode，在这一阶段，磁盘驱动尚未启动，需要采用特殊的方法获取代码。X86的ucore采用链接脚本实现，它把用户态程序的二进制拼接在系统内核之后，v9的编译器暂时不支持类似操作。我们使用的解决方法是将编译好的二进制文件当作磁盘映射到物理内存的最后4MB，因为模拟器通过类似的方式支持磁盘镜像。但是存在一个问题，在程序头中，没有存储文件的大小，所以我们编写了ChangeHeader.c小工具，重新构造程序的elfheader，使之包含所需的文件大小的信息。load_icode可以从物理内存的后4MB读取elfheader，然后根据提示的信息，把程序拷贝到新进程的内存中。

```
binary是二进制的虚地址指针，bsize是无效的参数
load_icode(unsigned char *binary, size_t bsize) 

  检查程序头
  if (hdr->magic != ELF_MAGIC) {
    ret = -E_INVAL_ELF;
    goto bad_elf_cleanup_pgdir;
  }

  为进程构建PDT	
  if (current->mm != NULL) {
    panic("load_icode: current->mm must be empty.\n");
  }
  if ((mm = mm_create()) == NULL) {
    goto bad_mm;
  }
  if (setup_pgdir(mm) != 0) {
    goto bad_pgdir_cleanup_mm;
  }

  vm_flags = 0, perm = PTE_U;
  vm_flags = VM_EXEC | VM_WRITE | VM_READ;
  if (vm_flags & VM_WRITE) perm |= PTE_W;

  if ((ret = mm_map(mm, USERBASE,  hdr->size + hdr->bss, vm_flags, NULL)) != 0) {
    goto bad_cleanup_mmap;
  }
  
  拷贝TEXT、DATA段
  from = (uint)(binary)+sizeof(struct elfhdr);
  start = USERBASE;
  la = ROUNDDOWN(start, PGSIZE);
  ret = -E_NO_MEM;
  end = USERBASE + hdr->size;
  while (start < end) {
    if ((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL) {
      goto bad_cleanup_mmap;
    }
    off = start - la, size = PGSIZE - off, la += PGSIZE;
    if (end < la) {
      size -= la - end;
    }
    memcpy(page2kva(page) + off, from, size);
    start += size, from += size;
  }

  构建BSS段
  end = USERBASE + hdr->size + hdr->bss;
  if (start < la) {
    /* ph->p_memsz == ph->p_filesz */
    if (start == end) {
      continue;
    }
    off = start + PGSIZE - la, size = PGSIZE - off;
    if (end < la) {
      size -= la - end;
    }
    memset(page2kva(page) + off, 0, size);
    start += size;
    assert((end < la && start == end) || (end >= la && start == la));
  }
  while (start < end) {
    if ((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL) {
      goto bad_cleanup_mmap;
    }
    off = start - la, size = PGSIZE - off, la += PGSIZE;
    if (end < la) {
      size -= la - end;
    }
    memset(page2kva(page) + off, 0, size);
    start += size;
  }
```

###Lab6-Lab7


###Lab8
Lab8加入了文件系统，由于ucore中的磁盘是从linux借鉴过来，具有复杂的层次结构，考虑到时间上的原因，我们没有修改这个复杂的结构，而是采用了xv6中的解决方案，在系统调用层面进行统一。相应的，还需要对load_icode进行有限的修改。

xv6的文件系统较为简单，对于一个文件，它可能有一个或多个inode进行描述，若文件小于1.9MB，那么inode中会有不超过480个地址，分别指向不同的4KB的页，存储相应的数据。对于超过1.9MB但是小于2GB的文件(对于ucore基本不存在)，会有多层inode进行描述。关于目录direct，它是一个特殊的文件。

为了支持系统调用，我们需要支持文件的open、alloc、read、close等操作，这些操作实现在file.h中。文件的操作会涉及到文件系统驱动的支持，文件系统驱动的主要功能是解析路径，找到文件在内存映射中的地址。

对于进程，需要新增当前路径pwd的支持。由于文件系统引入，不再需要changeHeader来提供额外的elfHeader信息，该信息可以由文件系统提供。

```
struct { 
 uint magic //0xC0DEF00D 文件的magic number 
 uint bss　 //在v9-cpu执行时没有用到
 uint entry //程序的执行入口地址
 uint flags;//程序的数据段起始地址 
} hdr;

0x00000000-------> +---------------------------------+
                   |                                 |
                   |             Header              |
                   |                                 |
                   +---------------------------------+
                   |                                 |
                   |              Text               |
                   |                                 |
                   +---------------------------------+
                   |                                 |
                   |              Data               |
                   |                                 |
                   +---------------------------------+

在changeHeader后变为

struct { 
 uint magic //0xC0DEF00D 文件的magic number 
 uint size;	//Text+Data段的大小
 uint bss　 //在v9-cpu执行时没有用到
 uint entry //程序的执行入口地址
 uint flags;//程序的数据段起始地址 
} hdr;

0x00000000-------> +---------------------------------+
                   |                                 |
                   |             Header              |
                   |                                 |
                   +---------------------------------+
                   |                                 |
                   |              Text               |
                   |                                 |
                   +---------------------------------+
                   |                                 |
                   |              Data               |
                   |                                 |
                   +---------------------------------+

bss段因为均为0，所以不需要专门的段来进行描述，data+text段之后的bss大小的部分全置0即可。

```

## 实验收获

####*	调试技巧

*	操作系统在调试时往往会有错误的发生处于造成错误的原因不匹配的情况，对于和底层相关的部分更是会出现这样的情况。调试的时候在找到出错点之后最好把过程中涉及到的链条逻辑都过一遍，看是否出现问题。尤其是在内存分配和进程切换这两部分中这尤为重要。


####*	代码阅读

*	代码阅读最好能够对整体的结构有一个很好的把握，尤其是各部分初始化的逻辑链条一定要清楚。阅读ucore代码时建议先从内部若干个小的库文件开始，一是整个操作系统会涉及到许许多多这些微小的分支，如果对这些库以及内部的函数没有基本的理解，主体的代码也是无从下手。


####*	收获

*	总的来说不小的工程量和简陋的cpu、编译器都给工作带来了巨大的困难，不过在解决这些困难之后能很好的对操作系统、底层硬件、编译器有一个整体的认识，并且整体的认识中间还是有血有肉的，实现一遍会让许多微小的在概念上很难想到的问题暴露出来。虽然需要耗费不少时间去解决这些问题，但耗费的时间还是非常有价值的。
