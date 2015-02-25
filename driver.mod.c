#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xcf79a31, "module_layout" },
	{ 0x234155d0, "cdev_alloc" },
	{ 0xadf42bd5, "__request_region" },
	{ 0xdc152fac, "cdev_del" },
	{ 0x15692c87, "param_ops_int" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x788fe103, "iomem_resource" },
	{ 0xfaaad20d, "device_destroy" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x7d11c268, "jiffies" },
	{ 0x27e1a049, "printk" },
	{ 0x8828f461, "fasync_helper" },
	{ 0xcc2e009e, "device_create" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0x331707cb, "cdev_add" },
	{ 0xc2165d85, "__arm_iounmap" },
	{ 0x9bce482f, "__release_region" },
	{ 0x7fe29886, "class_destroy" },
	{ 0x2c6540f, "kill_fasync" },
	{ 0x40a6f522, "__arm_ioremap" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xac8f37b2, "outer_cache" },
	{ 0x3949871f, "__class_create" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

