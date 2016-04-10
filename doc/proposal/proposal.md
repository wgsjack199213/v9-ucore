# 操作系统课程设计调研报告

**韩旭 徐磊    朱俸民\* 王奥丞\* 马坚鑫\***

## 目标
移植ucore代码（包含了lab1~lab8）到v9-cpu上。并进一步完善相关算法

- 连续内存分配算法first/best/worst/buddy system/slab
- 页面替换算法fifo/clock/advance clock/工作集/缺页率
- 改进ucore实验指导书
- *process/thread机制
- *改进pipe/named pipe/signal/sharemem IPC机制
- *spin lock/semaphore机制
- *FCFS/RoundRobin/MLFQ调度算法

## 相关材料
https://github.com/rswier/swieros

- 这个项目是v9的原型
- 提供了v9原始的编译器、模拟器、以及作者移植的xv9系统
- 没有提供标准的C编译器和文档

https://github.com/chyyuu/v9-cpu

- 陈渝老师修改过的v9-CPU
- 删去了原版中socket的支持
- 提供了CPU指令和寄存器更多的文档
- 提供了更多的示例程序

## xv6和ucore差异分析

**xv6**

- 分页机制，与x86不同，v9-cpu上没有段机制
- 中断，有基本的中断支持，目前看来可以支持ucore的功能
- 启动用户态程序，可以启动用户态程序
- 进程管理，有一个暴力的调度算法
- socket，底层提供了一些socket的机器指令，这部分功能我们准备删去
- 外部文件的访问，可以讲文件系统映射到到操作系统中

**ucore-lab1**

- 需要实现时钟中断，与xv6中的中断没有什么差异，应该可以很快的实现。

**ucore-lab2**

- 启动分页机制，与xv6中的分页没有什么差异，可以很快的实现。

**ucore-lab3**

- 实现虚存，可以现在内存中开一块静态内存空间作为swap区，实现一个假的swap，等之后磁盘支持后再切换过去。实现swap不存在大的技术困难，切换到磁盘可能会有一些困难。

**ucore-lab4**

- 内核进程和调度，xv6中没有创建内核线程的操作，需要进一步调研确定是否需要额外的支持。

**ucore-lab5**

- 创建用户进程，xv6中有相应的实现。

**ucore-lab6**

- 进程调度框架，xv6中的调度算法比较简陋，但是实现新的算法并不困难。

**ucore-lab7**

- 同步互斥，xv6中没有相关支持。

**ucore-lab8**

- 磁盘文件系统，准备让底层提供类似mips的load和store指令，来支持文件系统。文件系统在host上体现为一个二进制文件。文件的格式可以用比较简单的标准文件系统。实现磁盘和编译器、模拟器的实现密切相关，需要在实现时和相应小组进一步确认细节。

## 实现方法

- 阅读xv6代码，分析xv6已有的功能，和V9-CPU提供的支持
- 参考xv6，重新实现ucore
- 对模拟器和编译器提出额外支持的要求
- 计划
	- lab1~lab3 (1周)
	- lab4~lab6 (2周)
	- lab7~lab8 (2周)

