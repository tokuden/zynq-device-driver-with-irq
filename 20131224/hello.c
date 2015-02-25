#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_AUTHOR("Kimura");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello world driver");

static int devmajor = 77;
static char* devname = "helloworld";

static int hello_open(struct inode* inode, struct file* pfile)
{
	printk(KERN_INFO "%s called", __func__);
	return 0;
}

static int hello_release(struct inode* inode, struct file* pfile)
{
	printk(KERN_INFO "%s called", __func__);
	return 0;
}

static ssize_t hello_read(struct file* pfile, char* buf, size_t count, loff_t* pos)
{
	printk(KERN_INFO "%s(%pF) called", __func__, &hello_read);
	return 0;
}


static ssize_t hello_write(struct file* pfile, char* buf, size_t count, loff_t* pos)
{
	printk(KERN_INFO "%s(%pF) called", __func__, &hello_write);
	return 0;
}


static struct file_operations hello_fops = 
{
	owner	: THIS_MODULE,
	read	: hello_read,
	write	: hello_write,
	open	: hello_open,
	release	: hello_release,
};

static int hello_init(void)
{
	printk(KERN_INFO "driver loaded\n");
	printk(KERN_INFO "Hello, world!\n");
	/* register driver */
	if(register_chrdev(devmajor, devname, &hello_fops)){
		printk(KERN_ALERT "register_chrdev() failed");
		return -EBUSY;
	}

	printk(KERN_INFO "register_chrdev() succeeded");
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_INFO "driver unloaded\n");
	unregister_chrdev(devmajor, devname);
}

module_init(hello_init);
module_exit(hello_exit);

