
###实现改进的时钟页替换算法

在ucore/kern/mm/swap_fifo.h文件中，已经对FIFO页替换算法进行了实现。我仿照该文件创建了ucore/kern/mm/swap_enhclock.h文件，并在其中实现了改进的时钟页替换算法。实现中涉及几个要点如下：

- 首先要意识到FIFO页替换算法中用到的pra_list_head变量并不是FIFO队列中的第一个元素。一开始我根据惯性思维以为这个变量是指向队列第一个元素的指针，导致时钟页替换算法在运行时一直有错误（页虚拟地址不对）。经过重审代码才发现pra_list_head指向队列（双向链表实现）的头部，但是它并不指向第一个元素，pra_list_head->next才是队列中的第一个元素（同时pra_list_head->prev为队列尾端元素）。

- 改进的时钟页替换算法需要一个环形的数据结构。由于双向链表的性质我们可以简单地利用之前实现的FIFO队列来实现这个环。唯一需要注意的是环上并不是所有元素都是页，其中有一个元素是pra_list_head，所以我们在绕环遍历元素时，如果访问到了pra_list_head需要再往前跳一步。

- 改进的时钟页替换算法需要一个时钟指针，我修改了pmm.h中对于mm_struct结构体的定义，增加了一个list_entry_t类型的属性clock_hand。在初始化pra_list_head时同时初始化clock_hand。clock_hand旋转方向可以为顺时针也可以为逆时针，我任意选择了其中一个方向实现。

- 每次需要换出页时，检查时钟指针指向的页，通过get_pte函数获得该页的页表项，其中第6位和第7位标志位分别代表PTE_A（accessed）和PTE_D（dirty）。检查者两个标志位的值，并根据改进的时钟页替换算法进行值的更新，转动时钟指针，或者将该页进行换出操作即可。

###对原报告的一项修订

<https://github.com/leopard1/v9-ucore/blob/dev/README.md#---页缺失>中同学写的注释有点小问题，似乎是写反了，不应当是：

	如果二级页表本身已经存在，则将内存中该物理页中的数据复制到对应的交换分区的磁盘中，并将addr映射到内存相应的物理页上。

而应当是：

	如果页表项对应的页在交换分区中，则从磁盘上将数据载入到该页表项对应物理地址的页中，并调用page_insert建立逻辑地址和物理地址间的映射。



---

###实验中遇到的困难
我对于v9-ucore的改动并不多，但在实验过程中由于不熟悉v9自带的c编译器的一些性质，在一些编译错误上卡壳浪费了很多的时间（很简单的一个变量声明语句也会引发编译报错），经过一些摸索才发现可能是编译器的问题。最终我绕过这些编译错误的方式是尽量把每个函数体内所有变量的声明放在靠前的位置。下面附有我发现的几种会引发编译报错的样例代码，这些错误主要都是变量声明语句引起的。


####附：不能被编译器成功编译的若干样例程序

**样例程序1**（if中声明变量）：

	int main()
	{
	    int x = 1;
	    if (x == 0) {
	        int y;
	    }   
	    return 0;   
	}

编译输出：

	../xc : compiling if.c
	../xc : [if.c:5] error: bad expression
	fatal compiler error rv(int *a) *a=0
	
**样例程序2**（while中声明变量）：
	int main()
	{
	    int x = 1;
	    while (x >= 0) {
	        int y;
	        x = x - 1;
	    }   
	    return 0;   
	}
	
编译输出：

	../xc : compiling while.c
	../xc : [while.c:5] error: bad expression
	../xc : [while.c:5] error: ';' expected
	../xc : while.c compiled with 2 errors
	entry = 0 text = 64 data = 0 bss = 0
	../xc : exiting

**样例程序3**（调用函数后声明变量）：

	int f(int x) {
	    x = 2;
	}
	
	int main()
	{
	    int x = 1;
	    f(x);
	    int y;
	    return 0;   
	}


编译输出：

	../xc : compiling call.c
	../xc : [call.c:9] error: bad expression
	fatal compiler error rv(int *a) *a=0



