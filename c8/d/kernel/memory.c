#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "global.h"
#include "string.h"
#include "debug.h"
#include "bitmap.h"

#define PG_SIZE 4096

#define MEM_BITMAP_BASE 0xc009a000
#define K_HEAP_START    0xc0100000


#define PDE_IDX(addr)	((addr & 0xffc00000) >> 22)
#define PTE(addr)	((addr & 0x003ff000) >> 12)

/*内存池结构，生成两个实例用于管理内核内存池和用户内存池*/
struct pool
{
	struct bitmap pool_bitmap;	/*本内存池用到的位图结构，用于管理物理内存*/
	uint32_t phy_addr_start;	/*本内存池所管理物理内存的起始地址*/
	uint32_t pool_size;		/*本内存池字节容量*/
}

struct pool kernel_pool, user_pool;
struct virtual_addr kernel_vaddr;

/*初始化内存池*/
static void mem_pool_init(uint32_t all_mem)
{
	put_str("   mem_pool_init start\n");
	
	/*页表大小 = 1 页的页目录表 + 第 0 和第 768 个页目录项指向同一个页表 \
	+ 第 769 ~ 1022 个页目录项 = 256个页框*/
	uint32_t page_table_size = PG_SIZE * 256;
	
	uint32_t used_mem = page_table_size + 0x100000;
	uint32_t free_mem = all_mem - used_mem;
	uint16_t all_free_pages = free_mem / PG_SIZE;	

	uint16_t kernel_free_pages = all_free_pages / 2;
	uint16_t user_free_pages = all_free_pages - kernel_free_pages;

	uint32_t kbm_length = kernel_free_pages / 8;
	uint32_t ubm_length = user_free_pages / 8;

	uint32_t kp_start = used_mem;
	uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;

	kernel_pool.phy_addr_start = kp_start;
	user_pool.phy_addr_start = up_start;

	kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
	user_pool.pool_size = user_free_pages * PG_SIZE;

	kernel_pool.pool_bitmap.btmp_bytes_len =  kbm_length;
	user_pool.pool_bitmap.btmp_bytes_len = ubm_length;

	kernel_pool.pool_bitmap.bits = (void*)MEM_BITMAP_BASE;
	user_pool.pool_bitmap.bits = (void*)(MEM_BITMAP_BASE + kpm_legth);

	put_str("    kernel_pool_bitmap_start:");
	put_int((int)kernel_pool.pool_bitmap.bits);
	put_str(" kernel_pool_phy_addr_start:");
	put_int((int)kernel_pool.phy_addr_start);
	put_str("\n");
	put_str("user_pool_bitmap_start:");
	put_int((int)user_pool.pool_bitmap.bits);
	put_str(" user_pool_phy_addr_start:");
	put_int((int)user_pool.phy_addr_start);
	put_str("\n");

	bitmap_init(&kernel_pool.pool_bitmap);
	bitmap_init(&user_pool.pool_bitmap);

	kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;
	kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + kpm_length + ubm_length);
	kernel_vaddr.vaddr_start = K_HEAP_START;

	bitmap_init(&kernel_vaddr.vaddr_bitmap);
	put_str("    mem_pool_init done\n");
}

void mem_init()
{
	put_str("mem_init start\n");
	uint32_t mem_bytes_total = (*(uint32_t*)(0xb00));
	mem_pool_init(mem_bytes_total);
	put_str("mem_init done\n");
}

/*在pf表示的虚拟内存池中申请pg_cnt个虚拟页，成功则返回虚拟页的起始地址，失败则返回NULL*/
static void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt)
{
	int vaddr_start = 0, bit_idx_start = -1;
	uiunt32_t cnt = 0;
	if (pf == PF_KERNEL)
	{	
		bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
		if(bit_idx_start == -1)
			return NULL;
		while(cnt < pg_cnt)
			bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
		vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
	}
	else
	{
	}
	return (void*)vaddr_start;
}

/*得到虚拟地址vaddr对应的pte指针*/
uint32_t* pte_ptr(uint32_t vaddr)
{
	uint32_t* pte = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);0
	return pte;
}

/*得到虚拟地址vaddr对应的pde指针*/
uint32_t pde_ptr(uint32_t vaddr)
{
	uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr) * 4);
	return pde;
}

/*在m_pool指向的物理内存池中分配1个物理页，成功则返回页框的物理地址，失败则返回NULL*/
static void* palloc(struct pool* m_pool)
{
	int bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1);
	if(bit_idx == -1)
		return NULL;
	bitmap_set(&m_pool->pool_bitmap, bit_idx, 1);
	uint32_t page_phyaddr = ((bit_idx * PG_SIZE) + m_pool->phy_addr_start);
	return (void*)page_phyaddr;
}

