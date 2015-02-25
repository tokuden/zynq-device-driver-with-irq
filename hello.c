// Zynqで動作するLinux向けの、簡単なデバイスドライバです。
// 特殊電子回路のCosmo-Zや、DigilentのZedBoardで動作を確認しております。
// author	: Hideyuki Kimura
// license 	: GPL 
//
// 編集履歴
// 20150218		mknodを自動化するために、class_create()/device_create()などを追加
// 20150225		main()、hello_fasync()での返り値がない部分の抜けを修正。hello_fasync()内でfasync_helper()を呼ぶ前にmagicを見ない。fasyncをreleaseする
//
// TODO :
// ・デバイススペシャルファイルのchmod(しなくとも動くので必要ないかもしれない。)

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>		// class_create
#include <linux/sched.h>
#include <linux/interrupt.h>	// request_irq/free_irq
#include <linux/jiffies.h>		// jiffies
#include <linux/cdev.h>			// cdev_init/cdev_add/cdev_del
#include <linux/slab.h>			// kmalloc
#include "fpga_reg_util.h"


MODULE_AUTHOR("H.Kimura");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("adcdriver"); 

// constant variables
static char* DEV_NAME = "adcdriver";
// const int default_irq_number = 86;
static int irq_number = 86;						// my IRQ number. default IRQ number is #86. 

// global variables
dev_t allocated_device_number;
const unsigned int alloc_count = 1;				// デバイス数。複数ある場合をこのコードはまだ想定してない。
struct class* adc_class;
struct device* adc_device;
struct cdev *adc_cdev;
struct fasync_struct *p_fasync_struct = NULL;	// ポインタの実体は持つ。ポインタの参照先はなし。(fasync_helperで自動的に確保される)

module_param(irq_number, int, S_IRUGO);			// You can specify this when to start the driver.

static int hello_open(struct inode* inode, struct file* pfile)
{
	printk(KERN_INFO "%s(%pF) called at process %i (%s) \r\n", __func__, &hello_open, current->pid, current->comm);
	return 0;
}

static ssize_t hello_read(struct file* pfile, char* buf, size_t count, loff_t* pos)
{
	printk(KERN_INFO "%s(%pF) called at process %i (%s) \r\n", __func__, &hello_read, current->pid, current->comm);
	return 0;
}

static ssize_t hello_write(struct file* pfile, const char* buf, size_t count, loff_t* pos)
{
	printk(KERN_INFO "%s(%pF) called at process %i (%s) \r\n", __func__, &hello_write, current->pid, current->comm);

	return count;
}

static int hello_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	printk(KERN_INFO "%s(%pF) called at process %i (%s) \r\n", __func__, &hello_ioctl, current->pid, current->comm);

	switch (cmd){
	default:
		return -ENOTTY;
	}

	return 0;
}

static int hello_fasync(int fd, struct file *filp, int mode)
{
	printk(KERN_INFO "%s(%pF) called at process %i (%s) \r\n", __func__, &hello_fasync, current->pid, current->comm);

	const int ret_val = fasync_helper(fd, filp, mode, &p_fasync_struct);
	if(ret_val < 0){
		// error
		printk(KERN_WARNING "failed at fasync_helper()\r\n");
	} else if(ret_val == 0){
		printk(KERN_NOTICE "use old fasync_struct. no struct inserted to async list.\r\n");
	} else {
		printk(KERN_NOTICE "new fasync_struct allocated.\r\n");
	}

	return 0;		// 2015/02/25
}

static int hello_release(struct inode* inode, struct file* pfile)
{
	printk(KERN_INFO "%s(%pF) called at process %i (%s) \r\n", __func__, &hello_release, current->pid, current->comm);

	// ここでfasyncのflagを消す。(2015/02/25)
	const int ret_val = hello_fasync(-1, pfile, 0);

	return ret_val;
}

static struct file_operations hello_fops = 
{
	.owner			= THIS_MODULE,
	.read			= hello_read,
	.write			= hello_write,
	.open			= hello_open,
	.release		= hello_release,
	.unlocked_ioctl	= hello_ioctl,
	.fasync			= hello_fasync,
};


// Interrupt handler
static irqreturn_t test_interrupt_handler(int received_irq, void* dev_id)
{
	// 共有可能なドライバとするために、自分のドライバ以外の割込みならば、処理せずIRQ_NONEを返す。

	if (received_irq == irq_number){
		// 割り込みハンドラは極力早く処理を済ませるべき。今は問題ないが、printkもあまり良くない。
		printk("IRQ %d interrupted! dev_id \"%s\", jiffies %ld\r\n", received_irq, (char*)dev_id, jiffies);

		// 割込みが来たら、まずは割込みレジスタの中身を見る。
		const unsigned int reg_pushsw_int = read_fpga_register(0x20018 * sizeof(unsigned long));

		bool is_interrupt_from_adc = false;

		// どのプッシュスイッチからの割込みか確認する。
		if ((reg_pushsw_int & (1 << 0)) == (1 << 0)){
			printk(KERN_INFO "interrupt by pushsw(1)\r\n");	// SW1
		} else if ((reg_pushsw_int & (1 << 2)) == (1 << 2)){
			printk(KERN_INFO "interrupt by pushsw(2)\r\n");	// SW2
		} else if ((reg_pushsw_int & (1 << 4)) == (1 << 4)){
			printk(KERN_INFO "interrupt by pushsw(3)\r\n");	// SW3
		} else {
			printk(KERN_INFO "interrupted by ADC\r\n", reg_pushsw_int);	// ADC
			is_interrupt_from_adc = true;
		}

		// 割り込みが来たら、割り込みレジスタの割り込みフラグを'0'にして、割り込みをなくす。
		if (is_interrupt_from_adc){
			write_fpga_register(0x20019 * sizeof(unsigned long), (1 << 1));
			printk(KERN_INFO "%x : %08x\r\n", 0x20019 * sizeof(unsigned long), read_fpga_register(0x20019 * sizeof(unsigned long)));
		}
		else {
			write_fpga_register(0x20018 * sizeof(unsigned long), (1 << 1) | (1 << 3) | (1 << 5));
			printk(KERN_INFO "%x : %08x\r\n", 0x20018 * sizeof(unsigned long), read_fpga_register(0x20018 * sizeof(unsigned long)));
		}

		// 割込みが起きたので、ユーザープロセスに通知を送る
		if (p_fasync_struct){
			// magicを表示する(デバッグ)
			// printk(KERN_INFO "magic : %d\r\n", p_fasync_struct->magic);

			kill_fasync(&p_fasync_struct, SIGIO, POLL_IN);
		}

		return IRQ_HANDLED;
	} else {
		return IRQ_NONE;
	}
}

static int __init hello_init(void)
{
	// デバイス番号の割り当て(動的)
	const int result = alloc_chrdev_region(&allocated_device_number, 0, alloc_count, DEV_NAME);

	if (result < 0){
		printk(KERN_WARNING "can't get major number %d\r\n", MAJOR(allocated_device_number));
		return result;
	}

	// 自動的にmknodする(2015/02/18)
	adc_class = class_create(THIS_MODULE, DEV_NAME);
	if(IS_ERR(adc_class)){
		printk(KERN_WARNING "failed at class_create()\r\n");

		unregister_chrdev_region(allocated_device_number, alloc_count);
		
		return -1;
	}

	// mknodのためのデバイス(2015/02/18)
	adc_device = device_create(adc_class, NULL, allocated_device_number, NULL, DEV_NAME);
	if(IS_ERR(adc_device)){
		printk(KERN_WARNING "failed at device_create()\r\n");
		
		class_destroy(adc_class);
		unregister_chrdev_region(allocated_device_number, alloc_count);
		
		return -1;
	}

	// ドライバを登録する。cdev_init()でなくともよい。
	// エラーチェック追加(2015/02/18)
	adc_cdev = cdev_alloc();
	if(adc_cdev == NULL){
		printk(KERN_WARNING "failed at cdev_alloc()\r\n");
		
		class_destroy(adc_class);
		unregister_chrdev_region(allocated_device_number, alloc_count);
		
		return -1;
	}

	adc_cdev->ops	= &hello_fops;
	adc_cdev->owner	= THIS_MODULE;

	const int err_code = cdev_add(adc_cdev, allocated_device_number, 1);
	if (err_code){
		printk(KERN_WARNING "Error %d adding %s\r\n", err_code, DEV_NAME);

		device_destroy(adc_class, allocated_device_number);
		class_destroy(adc_class);
		unregister_chrdev_region(allocated_device_number, alloc_count);

		return -EBUSY;
	} else {
		printk(KERN_NOTICE "We successfully added this device as %d %d\r\n", MAJOR(allocated_device_number), MINOR(allocated_device_number));
	}

	// 割り込みハンドラの登録
	printk(KERN_INFO "install %s into irq %d\n", DEV_NAME, irq_number);

	// SA_SHIRQ → IRQF_SHARED、SA_INTERRUPT → IRQF_DISABLED
	if (request_irq(irq_number, test_interrupt_handler, IRQF_SHARED, DEV_NAME, DEV_NAME))
	{		
		// 登録できない時の終了処理
		printk(KERN_WARNING "failed at request_irq(), unregister driver (devnum : %d, DEV_NAME : %s)\r\n", allocated_device_number, DEV_NAME);

		cdev_del(adc_cdev);
		device_destroy(adc_class, allocated_device_number);
		class_destroy(adc_class);
		unregister_chrdev_region(allocated_device_number, alloc_count);
		
		return -EBUSY;
	}

	// 割り込みレジスタの割り込み許可フラグ(INT_EN)を立てて、割込めるようにしておく。
	map_fpga_register(XPAR_AXI_EXT_SLAVE_CONN_0_S_AXI_RNG00_BASEADDR, 0x20100 * sizeof(unsigned long));
	
	write_fpga_register(0x20018 * sizeof(unsigned long), ((1 << 1) | (1 << 3) | (1 << 5)));			// 0x2a
	write_fpga_register(0x20019 * sizeof(unsigned long), (1 << 1));
	print_fpga_registers();

	// 非同期通知用の構造体の確保(ここでやる必要ある？)
	// p_fasync_struct = NULL;	// (struct fasync_struct*)kmalloc(sizeof(struct fasync_struct), GFP_ATOMIC);
	// if(p_fasync_struct == NULL){
	// 	printk(KERN_WARNING "failed at kmalloc()\r\n");

	// 	free_irq(irq_number, DEV_NAME);
	// 	cdev_del(adc_cdev);
	// 	device_destroy(adc_class, allocated_device_number);
	// 	class_destroy(adc_class);
	// 	unregister_chrdev_region(allocated_device_number, alloc_count);

	// 	return -ENOMEM;
	// } else {
	// 	// memset(p_fasync_struct, 0, sizeof(struct fasync_struct));		// Zero clear
	// 	// p_fasync_struct->magic = 

	// 	// 2015/01/13
	// 	// magicを表示する(デバッグ)
	// 	// printk(KERN_INFO "magic : %d\r\n", p_fasync_struct->magic);
	// 	// printk(KERN_INFO "Address of p_fasync_struct is %x\r\n", &p_fasync_struct);
	// }
	return 0;
}

static void __exit hello_exit(void)
{
	// 割込みハンドラの登録を解除する。
	printk(KERN_INFO "removing %s from irq %d\n", DEV_NAME, irq_number);
	free_irq(irq_number, DEV_NAME);

	// 割り込むこともないので、割込み許可フラグを0にする。
	write_fpga_register(0x20018 * sizeof(unsigned long), 0x00000000);
	write_fpga_register(0x20019 * sizeof(unsigned long), 0x00000000);
	unmap_fpga_register(XPAR_AXI_EXT_SLAVE_CONN_0_S_AXI_RNG00_BASEADDR, 0x20100 * sizeof(unsigned long));

	// 非同期通知用構造体の確保してたメモリを開放
	// kfree(p_fasync_struct);
	
	// ドライバの登録を解除する。
	cdev_del(adc_cdev);

	// mknodのために作成したclassも消す
	device_destroy(adc_class, allocated_device_number);
	class_destroy(adc_class);

	// デバイスの登録も消す。(2015/02/25)
	unregister_chrdev_region(allocated_device_number, alloc_count);

	printk(KERN_INFO "We successfully unregisterd driver (DEV_NAME : %s)\n", DEV_NAME);
}

module_init(hello_init);
module_exit(hello_exit);