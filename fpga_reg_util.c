#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <asm/io.h>				// ioremap
// #include <asm/system.h>			// wmb
								// removed http://superuser.com/questions/783285/asm-system-h-header-file-linux
#include <linux/ioport.h>		// request_mem_region
#include "fpga_reg_util.h"

void* __iomem mapped_fpga_reg_addr;

// 物理アドレスでaddrから、sizeだけ、I/Oメモリを割り当てます。
// 1回呼び、成功した場合は、マップした仮想アドレスをmapped_fpga_reg_addrに代入します。
// 読み書きには下にある関数を使います。
// 終了時にunmap_fpga_register()を呼び、メモリを解放してください。
void map_fpga_register(const unsigned long addr, const unsigned long size)
{
	char name[] = "ADC decoder";

	// I/Oメモリ割り当て
	if (!request_mem_region(addr, size, name)){
		printk(KERN_INFO "failed at request_mem_region()\r\n");
		return;
	}

	// ioremap()で、物理アドレスを仮想アドレス空間にマッピングする。
	mapped_fpga_reg_addr = ioremap(addr, size);
	if (mapped_fpga_reg_addr == NULL){
		printk(KERN_WARNING "failed at ioremap()\r\n");
	} else {
		printk(KERN_INFO "specified addr %08x - %08x mapped to %08x - %08x\r\n", addr, addr + size, mapped_fpga_reg_addr, mapped_fpga_reg_addr + size);
	}
}


// offset addr = physical addr - regbase addr
unsigned int read_fpga_register(const unsigned long offset)
{
	if (mapped_fpga_reg_addr == NULL){
		printk(KERN_WARNING "mapped_fpga_reg_addr == NULL at %s(%pF) PID %i\r\n", __func__, &read_fpga_register, current->pid);
		return 0xdeadbeaf;
	} else {
		return ioread32(mapped_fpga_reg_addr + offset);
	}
}

// offset addr = physical addr - regbase addr
void write_fpga_register(const unsigned long offset, const unsigned long value)
{
	if (mapped_fpga_reg_addr == NULL){
		printk(KERN_WARNING "mapped_fpga_reg_addr == NULL at %s(%pF) PID %i\r\n", __func__, &write_fpga_register, current->pid);
	} else {
		iowrite32(value, mapped_fpga_reg_addr + offset);
		wmb();
	}
}

void unmap_fpga_register(const unsigned long addr, const unsigned long size)
{
	if(mapped_fpga_reg_addr == NULL){
		printk(KERN_WARNING "mapped_fpga_reg_addr == NULL at %s(%pF) PID %i\r\n", __func__, &unmap_fpga_register, current->pid);
	} else {
		iounmap(mapped_fpga_reg_addr);
		release_mem_region(addr, size);
	}
}

// Utility関数です。FPGAにあるレジスタのうち、start～endまでをダンプします。
// 15/01/13 fixed : -Werror=strict-prototype () -> (void)
void print_fpga_registers(void)
{
	const unsigned long start = 0x20000;
	const unsigned long end = 0x20018;
	unsigned long i = start;

	for (i = start; i <= end; i++){
		printk("%08x : %08x\r\n", XPAR_AXI_EXT_SLAVE_CONN_0_S_AXI_RNG00_BASEADDR + i, read_fpga_register(i * sizeof(unsigned long)));
	}
}