#ifndef __KERN_MM_PMM_H__
#define __KERN_MM_PMM_H__

#include <defs.h>
#include <mmu.h>
#include <memlayout.h>
#include <atomic.h>
#include <io.h>
#include <call.h>
#include <sync.h>

#define alloc_page() alloc_pages(1)
#define free_page(page) free_pages(page, 1)

struct Page *pages;
// amount of physical memory (in pages)
size_t npage = 0;

// virtual address of boot-time page directory
pde_t *boot_pgdir = NULL;
// physical address of boot-time page directory
uintptr_t boot_cr3;
uint endbss;

// /* *
//  * PADDR - takes a kernel virtual address (an address that points above KERNBASE),
//  * where the machine's maximum 256MB of physical memory is mapped and returns the
//  * corresponding physical address.  It panics if you pass it a non-kernel virtual address.
//  * */
// #define PADDR(kva) ({                                                   \
//             uintptr_t __m_kva = (uintptr_t)(kva);                       \
//             if (__m_kva < KERNBASE) {                                   \
//                 panic("PADDR called with invalid kva %08lx", __m_kva);  \
//             }                                                           \
//             __m_kva - KERNBASE;                                         \
//         })

// /* *
//  * KADDR - takes a physical address and returns the corresponding kernel virtual
//  * address. It panics if you pass an invalid physical address.
//  * */
// #define KADDR(pa) ({                                                    \
//             uintptr_t __m_pa = (pa);                                    \
//             size_t __m_ppn = PPN(__m_pa);                               \
//             if (__m_ppn >= npage) {                                     \
//                 panic("KADDR called with invalid pa %08lx", __m_pa);    \
//             }                                                           \
//             (void *) (__m_pa + KERNBASE);                               \
//         })

/* *
 * PADDR - takes a kernel virtual address (an address that points above KERNBASE),
 * where the machine's maximum 256MB of physical memory is mapped and returns the
 * corresponding physical address.  It panics if you pass it a non-kernel virtual address.
 * */
uint PADDR(uint kva) {
    uint __m_kva;
    __m_kva = kva;
    if (__m_kva < KERNBASE) {
        panic("PADDR called with invalid kva %x\n", __m_kva);
    }
    return __m_kva - KERNBASE;
}

/* *
 * KADDR - takes a physical address and returns the corresponding kernel virtual
 * address. It panics if you pass an invalid physical address.
 * */
uint KADDR(uint pa) {
    uint __m_pa;
    uint __m_ppn;
    __m_pa = pa;
    __m_ppn = PPN(__m_pa);
    if (__m_ppn >= npage) {
                panic("KADDR called with invalid pa %x", __m_pa);
    }
    return __m_pa + KERNBASE;
}

ppn_t
page2ppn(struct Page *page) {
    return page - pages;
}

uintptr_t
page2pa(struct Page *page) {
    return page2ppn(page) << PGSHIFT;
}

struct Page *
pa2page(uintptr_t pa) {
    if (PPN(pa) >= npage) {
        panic("pa2page called with invalid pa");
    }
    return &pages[PPN(pa)];
}

void *
page2kva(struct Page *page) {
    return KADDR(page2pa(page));
}

struct Page *
kva2page(void *kva) {
    return pa2page(PADDR(kva));
}

struct Page *
pte2page(pte_t pte) {
    if (!(pte & PTE_P)) {
        panic("pte2page called with invalid pte");
    }
    return pa2page(PTE_ADDR(pte));
}

struct Page *
pde2page(pde_t pde) {
    return pa2page(PDE_ADDR(pde));
}

int
page_ref(struct Page *page) {
    return page->ref;
}

void
set_page_ref(struct Page *page, int val) {
    page->ref = val;
}

int
page_ref_inc(struct Page *page) {
    page->ref += 1;
    return page->ref;
}

int
page_ref_dec(struct Page *page) {
    page->ref -= 1;
    return page->ref;
}

pte_t *  vpt = (pte_t *)(VPT);
pde_t *  vpd = (pde_t *)PGADDR(PDX(VPT), PDX(VPT), 0);


// pmm_manager is a physical memory management class. A special pmm manager - XXX_pmm_manager
// only needs to implement the methods in pmm_manager class, then XXX_pmm_manager can be used
// by ucore to manage the total physical memory space.
struct pmm_manager {
    char *name;                                 // XXX_pmm_manager's name
    void* init;
    void* init_memmap;
    void* alloc_pages;
    void* free_pages;
    void* nr_free_pages;
    void* check;
};

void load_default_pmm_manager();

struct pmm_manager* pmm_manager;
struct pmm_manager default_pmm_manager;

//init_pmm_manager - initialize a pmm_manager instance
void
init_pmm_manager(void) {
    load_default_pmm_manager();
    pmm_manager = &default_pmm_manager;
    printf("memory management: %s\n", pmm_manager->name);
    call0(pmm_manager->init);
}

//init_memmap - call pmm->init_memmap to build Page struct for free memory
void
init_memmap(struct Page *base, size_t n) {
    call2(base, n, pmm_manager->init_memmap);
}

//alloc_pages - call pmm->alloc_pages to allocate a continuous n*PAGESIZE memory
struct Page *
alloc_pages(size_t n) {
    struct Page *page=NULL;
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        call1(n, pmm_manager->alloc_pages);
    }
    local_intr_restore(intr_flag);
    return page;
}

//free_pages - call pmm->free_pages to free a continuous n*PAGESIZE memory
void
free_pages(struct Page *base, size_t n) {
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        call2(base, n, pmm_manager->free_pages);
    }
    local_intr_restore(intr_flag);
}

//nr_free_pages - call pmm->nr_free_pages to get the size (nr*PAGESIZE) of current free memory
size_t
nr_free_pages(void) {
    size_t ret;
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        ret = call0(); 
    }
    local_intr_restore(intr_flag);
    return ret;
}

// pmm_init - initialize the physical memory management
static void
page_init(void) {
    uintptr_t freemem;
    uint32_t maxpa, begin, end;
    int i;
    maxpa = msiz();
    if (maxpa > KMEMSIZE) {
        maxpa = KMEMSIZE;
    }
    npage = maxpa / PGSIZE;
    pages = (struct Page *)ROUNDUP((uint)(endbss) + PGSIZE + 3, PGSIZE);
    printf("%d\n", pages );
    for (i = 0; i < npage; i ++) {
        SetPageReserved(pages + i);
    }
    freemem = PADDR((uintptr_t)pages + sizeof(struct Page) * npage);
    begin = freemem;
    end = maxpa;
    if (begin < end) {
        begin = ROUNDUP(begin, PGSIZE);
        end = ROUNDDOWN(end, PGSIZE);
        if (begin < end)
                    init_memmap(pa2page(begin), (end - begin) / PGSIZE);
    }
}

static void
enable_paging() {
    pdir(boot_cr3);
    spage(1);
}

/*
boot_alloc_page - allocate one page using pmm->alloc_pages(1)
return value: the kernel virtual address of this allocated page
note: this function is used to get the memory for PDT(Page Directory Table)&PT(Page Table)
*/
void *
boot_alloc_page(void) {
    struct Page *p = alloc_page();
    if (p == NULL) {
        panic("boot_alloc_page failed.\n");
    }
    return page2kva(p);
}

/*
    get_pte 

        - get pte and return the kernel virtual address of this pte for la
        - if the PT contians this pte didn't exist, alloc a page for PT

    parameter:

        pgdir:  the kernel virtual base address of PDT
        la:     the linear address need to map
        create: a logical value to decide if alloc a page for PT

    return vaule: the kernel virtual address of this pte
*/

pte_t *
get_pte(pde_t *pgdir, uintptr_t la, bool create) {
    pde_t *pdep = &pgdir[PDX(la)];
    struct Page *page;
    uintptr_t pa;
    /* LAB2 EXERCISE 2: YOUR CODE
     *
     * If you need to visit a physical address, please use KADDR()
     * please read pmm.h for useful macros
     *
     * Maybe you want help comment, BELOW comments can help you finish the code
     *
     * Some Useful MACROs and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   PDX(la) = the index of page directory entry of VIRTUAL ADDRESS la.
     *   KADDR(pa) : takes a physical address and returns the corresponding kernel virtual address.
     *   set_page_ref(page,1) : means the page be referenced by one time
     *   page2pa(page): get the physical address of memory which this (struct Page *) page  manages
     *   struct Page * alloc_page() : allocation a page
     *   memset(void *s, char c, size_t n) : sets the first n bytes of the memory area pointed by s
     *                                       to the specified value c.
     * DEFINEs:
     *   PTE_P           0x001                   // page table/directory entry flags bit : Present
     *   PTE_W           0x002                   // page table/directory entry flags bit : Writeable
     *   PTE_U           0x004                   // page table/directory entry flags bit : User can access
        #if 0
            pde_t *pdep = NULL;   // (1) find page directory entry
            if (0) {              // (2) check if entry is not present
                                  // (3) check if creating is needed, then alloc page for page table
                                  // CAUTION: this page is used for page table, not for common data page
                                  // (4) set page reference
                uintptr_t pa = 0; // (5) get linear address of page
                                  // (6) clear page content using memset
                                  // (7) set page directory entry's permission
            }
            return NULL;          // (8) return page table entry
        #endif
     */

    if (!(*pdep & PTE_P)) {
        if (!create || (page = alloc_page()) == NULL) {
            return NULL;
        }
        set_page_ref(page, 1);
        pa = page2pa(page);
        memset(KADDR(pa), 0, PGSIZE);
        *pdep = pa | PTE_U | PTE_W | PTE_P;
    }
    return &((pte_t *)KADDR(PDE_ADDR(*pdep)))[PTX(la)];
}

//get_page - get related Page struct for linear address la using PDT pgdir
struct Page *
get_page(pde_t *pgdir, uintptr_t la, pte_t **ptep_store) {
    pte_t *ptep = get_pte(pgdir, la, 0);
    if (ptep_store != NULL) {
        *ptep_store = ptep;
    }
    if (ptep != NULL && *ptep & PTE_P) {
        return pte2page(*ptep);
    }
    return NULL;
}

/*
page_remove_pte - free an Page sturct which is related linear address la
               - and clean(invalidate) pte which is related linear address la
note: PT is changed, so the TLB need to be invalidate
*/
void
page_remove_pte(pde_t *pgdir, uintptr_t la, pte_t *ptep) {
    /* LAB2 EXERCISE 3: YOUR CODE
     *
     * Please check if ptep is valid, and tlb must be manually updated if mapping is updated
     *
     * Maybe you want help comment, BELOW comments can help you finish the code
     *
     * Some Useful MACROs and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   struct Page *page pte2page(*ptep): get the according page from the value of a ptep
     *   free_page : free a page
     *   page_ref_dec(page) : decrease page->ref. NOTICE: ff page->ref == 0 , then this page should be free.
     *   tlb_invalidate(pde_t *pgdir, uintptr_t la) : Invalidate a TLB entry, but only if the page tables being
     *                        edited are the ones currently in use by the processor.
     * DEFINEs:
     *   PTE_P           0x001                   // page table/directory entry flags bit : Present
     */
// #if 0
//     if (0) {                      //(1) check if page directory is present
//         struct Page *page = NULL; //(2) find corresponding page to pte
//                                   //(3) decrease page reference
//                                   //(4) and free this page when page reference reachs 0
//                                   //(5) clear second page table entry
//                                   //(6) flush tlb
//     }
// #endif
    struct Page * page;
    if (*ptep & PTE_P) {
        page = pte2page(*ptep);
        if (page_ref_dec(page) == 0) {
            free_page(page);
        }
        *ptep = 0;
    }
}

//page_remove - free an Page which is related linear address la and has an validated pte
void
page_remove(pde_t *pgdir, uintptr_t la) {
    pte_t *ptep = get_pte(pgdir, la, 0);
    if (ptep != NULL) {
        page_remove_pte(pgdir, la, ptep);
    }
}

/*
    page_insert - build the map of phy addr of an Page with the linear addr la

    paramemters:
        pgdir: the kernel virtual base address of PDT
        page:  the Page which need to map
        la:    the linear address need to map
        perm:  the permission of this Page which is setted in related pte

    return value: always 0
    note: PT is changed, so the TLB need to be invalidate
*/
int
page_insert(pde_t *pgdir, struct Page *page, uintptr_t la, uint32_t perm) {
    pte_t *ptep = get_pte(pgdir, la, 1);
    struct Page *p;
    if (ptep == NULL) {
        // return -E_NO_MEM;
    }
    page_ref_inc(page);
    if (*ptep & PTE_P) {
        p = pte2page(*ptep);
        if (p == page) {
            page_ref_dec(page);
        }
        else {
            page_remove_pte(pgdir, la, ptep);
        }
    }
    *ptep = page2pa(page) | PTE_P | perm;
    return 0;
}

static void
check_alloc_page(void) {
    call0(pmm_manager->check);
    printf("check_alloc_page() succeeded!\n");
}

static void
check_pgdir(void) {
    struct Page *p1, *p2;
    pte_t *ptep;

    assert(npage <= KMEMSIZE / PGSIZE);
    assert(boot_pgdir != NULL && (uint32_t)PGOFF(boot_pgdir) == 0);
    assert(get_page(boot_pgdir, 0x0, NULL) == NULL);

    p1 = alloc_page();
    assert(page_insert(boot_pgdir, p1, 0x0, 0) == 0);
    
    assert((ptep = get_pte(boot_pgdir, 0x0, 0)) != NULL);
    assert(pte2page(*ptep) == p1);
    assert(page_ref(p1) == 1);

    ptep = &((pte_t *)KADDR(PDE_ADDR(boot_pgdir[0])))[1];
    assert(get_pte(boot_pgdir, PGSIZE, 0) == ptep);

    p2 = alloc_page();
    assert(page_insert(boot_pgdir, p2, PGSIZE, PTE_U | PTE_W) == 0);
    assert((ptep = get_pte(boot_pgdir, PGSIZE, 0)) != NULL);
    assert(*ptep & PTE_U);
    assert(*ptep & PTE_W);
    assert(boot_pgdir[0] & PTE_U);
    assert(page_ref(p2) == 1);

    assert(page_insert(boot_pgdir, p1, PGSIZE, 0) == 0);
    assert(page_ref(p1) == 2);
    assert(page_ref(p2) == 0);
    assert((ptep = get_pte(boot_pgdir, PGSIZE, 0)) != NULL);
    assert(pte2page(*ptep) == p1);
    assert((*ptep & PTE_U) == 0);

    page_remove(boot_pgdir, 0x0);
    assert(page_ref(p1) == 1);
    assert(page_ref(p2) == 0);

    page_remove(boot_pgdir, PGSIZE);
    assert(page_ref(p1) == 0);
    assert(page_ref(p2) == 0);

    assert(page_ref(pde2page(boot_pgdir[0])) == 1);
    free_page(pde2page(boot_pgdir[0]));
    boot_pgdir[0] = 0;

    printf("check_pgdir() succeeded!\n");
}

/*
    boot_map_segment - setup&enable the paging mechanism
    parameters
        la:   linear address of this memory need to map (after x86 segment map)
        size: memory size
        pa:   physical address of this memory
        perm: permission of this memory
*/

static void
boot_map_segment(pde_t *pgdir, uintptr_t la, size_t size, uintptr_t pa, uint32_t perm) {
    size_t n = ROUNDUP(size + PGOFF(la), PGSIZE) / PGSIZE;
    pte_t * ptep;
    assert(PGOFF(la) == PGOFF(pa));
    la = ROUNDDOWN(la, PGSIZE);
    pa = ROUNDDOWN(pa, PGSIZE);
    for (; n > 0; n --, la += PGSIZE, pa += PGSIZE) {
        ptep = get_pte(pgdir, la, 1);
        assert(ptep != NULL);
        *ptep = pa | PTE_P | perm;
    }
}

static void
check_boot_pgdir(void) {
    pte_t *ptep;
    int i;
    struct Page *p;
    char *str = "ucore: Hello world!!";

    for (i = 0; i < npage; i += PGSIZE) {
        assert((ptep = get_pte(boot_pgdir, (uintptr_t)KADDR(i), 0)) != NULL);
        assert(PTE_ADDR(*ptep) == i);
    }

    assert(PDE_ADDR(boot_pgdir[PDX(VPT)]) == PADDR(boot_pgdir));

    assert(boot_pgdir[0] == 0);

    p = alloc_page();
    assert(page_insert(boot_pgdir, p, 0x100, PTE_W) == 0);
    assert(page_ref(p) == 1);
    assert(page_insert(boot_pgdir, p, 0x100 + PGSIZE, PTE_W) == 0);
    assert(page_ref(p) == 2);

    
    strcpy((void *)0x100, str);
    assert(strcmp((void *)0x100, (void *)(0x100 + PGSIZE)) == 0);

    *(char *)(page2kva(p) + 0x100) = '\0';
    assert(strlen((char *)0x100) == 0);

    free_page(p);
    free_page(pde2page(boot_pgdir[0]));
    boot_pgdir[0] = 0;

    printf("check_boot_pgdir() succeeded!\n");
}

//perm2str - use string 'u,r,w,-' to present the permission
char *
perm2str(int perm) {
    char str[4];
    str[0] = (perm & PTE_U) ? 'u' : '-';
    str[1] = 'r';
    str[2] = (perm & PTE_W) ? 'w' : '-';
    str[3] = '\0';
    return str;
}

/*
    get_pgtable_items - In [left, right] range of PDT or PT, find a continuous linear addr space
                     - (left_store*X_SIZE~right_store*X_SIZE) for PDT or PT
                     - X_SIZE=PTSIZE=4M, if PDT; X_SIZE=PGSIZE=4K, if PT
    paramemters:
        left:        no use ???
        right:       the high side of table's range
        start:       the low side of table's range
        table:       the beginning addr of table
        left_store:  the pointer of the high side of table's next range
        right_store: the pointer of the low side of table's next range
    
    return value: 0 - not a invalid item range, perm - a valid item range with perm permission
*/
static int
get_pgtable_items(size_t left, size_t right, size_t start, uintptr_t *table, size_t *left_store, size_t *right_store) {
    int perm;
    if (start >= right) {
        return 0;
    }
    while (start < right && !(table[start] & PTE_P)) {
        start ++;
    }
    if (start < right) {
        if (left_store != NULL) {
            *left_store = start;
        }
        perm = (table[start ++] & PTE_USER);
        while (start < right && (table[start] & PTE_USER) == perm) {
            start ++;
        }
        if (right_store != NULL) {
            *right_store = start;
        }
        return perm;
    }
    return 0;
}


//print_pgdir - print the PDT&PT
void
print_pgdir(void) {
    size_t left, right = 0, perm;
    size_t l, r;
    printf("-------------------- BEGIN --------------------\n");
    while ((perm = get_pgtable_items(0, NPDEENTRY, right, vpd, &left, &right)) != 0) {
        printf("PDE(%03x) %08x-%08x %08x %s\n", right - left,
                left * PTSIZE, right * PTSIZE, (right - left) * PTSIZE, perm2str(perm));
        r = left * NPTEENTRY;
        while ((perm = get_pgtable_items(left * NPTEENTRY, right * NPTEENTRY, r, vpt, &l, &r)) != 0) {
            printf("  |-- PTE(%05x) %08x-%08x %08x %s\n", r - l,
                    l * PGSIZE, r * PGSIZE, (r - l) * PGSIZE, perm2str(perm));
        }
    }
    printf("--------------------- END ---------------------\n");
}

//pmm_init - setup a pmm to manage physical memory, build PDT&PT to setup paging mechanism
//         - check the correctness of pmm & paging mechanism, print PDT&PT
void
pmm_init(void) {
    //We need to alloc/free the physical memory (granularity is 4KB or other size).
    //So a framework of physical memory manager (struct pmm_manager)is defined in pmm.h
    //First we should init a physical memory manager(pmm) based on the framework.
    //Then pmm can alloc/free the physical memory.
    //Now the first_fit/best_fit/worst_fit/buddy_system pmm are available.

    init_pmm_manager();
    // detect physical memory space, reserve already used memory,
    // then use pmm->init_memmap to create free page list
    page_init();
    //use pmm->check to verify the correctness of the alloc/free function in a pmm
    check_alloc_page();

    // create boot_pgdir, an initial page directory(Page Directory Table, PDT)
    boot_pgdir = boot_alloc_page();
    memset(boot_pgdir, 0, PGSIZE);
    boot_cr3 = PADDR(boot_pgdir);

    check_pgdir();

    // static_assert(KERNBASE % PTSIZE == 0 && KERNTOP % PTSIZE == 0);

    // recursively insert boot_pgdir in itself
    // to form a virtual page table at virtual address VPT
    boot_pgdir[PDX(VPT)] = PADDR(boot_pgdir) | PTE_P | PTE_W;

    // map all physical memory to linear memory with base linear addr KERNBASE
    //linear_addr KERNBASE~KERNBASE+KMEMSIZE = phy_addr 0~KMEMSIZE
    //But shouldn't use this map until enable_paging() & gdt_init() finished.
    boot_map_segment(boot_pgdir, KERNBASE, KMEMSIZE, 0, PTE_W);

    //temporary map:
    //virtual_addr 3G~3G+4M = linear_addr 0~4M = linear_addr 3G~3G+4M = phy_addr 0~4M
    boot_pgdir[0] = boot_pgdir[PDX(KERNBASE)];

    enable_paging();

    //reload gdt(third time,the last time) to map all physical memory
    //virtual_addr 0~4G=liear_addr 0~4G
    //then set kernel stack(ss:esp) in TSS, setup TSS in gdt, load TSS

    //disable the map of virtual_addr 0~4M
    boot_pgdir[0] = 0;

    //now the basic virtual memory map(see memalyout.h) is established.
    //check the correctness of the basic virtual memory map.
    check_boot_pgdir();

    print_pgdir();

}

#endif /* !__KERN_MM_PMM_H__ */

