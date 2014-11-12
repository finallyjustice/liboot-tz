#include <linux/module.h>  
#include <linux/kernel.h>

static int __init hello_init(void)
{
	int i;
	printk(KERN_ALERT "Test SMC\n");

	for(i=0; i<10; i++)
	{
		printk(KERN_ALERT "Hello you are in normal kernel!\n");
		asm volatile(
			".arch_extension sec\n\t"
			"smc #0\n\t");
	}

	printk(KERN_ALERT "OK we are here!\n");

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

