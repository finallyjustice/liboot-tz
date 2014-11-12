#include <linux/module.h>  
#include <linux/kernel.h>

static int __init hello_init(void)
{
	printk(KERN_ALERT "Test TrustZone NS-bit\n");

	printk(KERN_ALERT "HELLO WORLD 0\n");
	printk(KERN_ALERT "HELLO WORLD 1\n");
	printk(KERN_ALERT "HELLO WORLD 2\n");
	printk(KERN_ALERT "HELLO WORLD 3\n");
	printk(KERN_ALERT "HELLO WORLD 4\n");
	printk(KERN_ALERT "HELLO WORLD 5\n");
	printk(KERN_ALERT "HELLO WORLD 6\n");
	printk(KERN_ALERT "HELLO WORLD 7\n");
	printk(KERN_ALERT "HELLO WORLD 8\n");
	printk(KERN_ALERT "HELLO WORLD 9\n");
	printk(KERN_ALERT "HELLO WORLD 10\n");
	printk(KERN_ALERT "HELLO WORLD 11\n");
	printk(KERN_ALERT "HELLO WORLD 12\n");
	printk(KERN_ALERT "HELLO WORLD 13\n");

	int scr_val = 1;

	__asm__ volatile(
			"mrc p15, 0, r0, c1, c1, 0\n\t"
			"mov %0, r0\n\t"
			: "=r"(scr_val)
			: 
			: "r0"
	);

	if(scr_val & 0x1)
		printk(KERN_ALERT "TrustZone NS Bit is 1 - Normal\n");
	else
		printk(KERN_ALERT "TrustZone NS Bit is 0 - Secure\n");

	printk(KERN_ALERT "SCR Reg: 0x%x\n", scr_val);

	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_ALERT "Remove the module\n");
}

module_init(hello_init);  
module_exit(hello_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dongli Zhang");
MODULE_DESCRIPTION("Android Kernel Module Hello World");

