#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/time.h>

static int __init ktime_init(void)
{
	printk(KERN_ALERT "init the module\n");
	
	struct timespec ts1;
	struct timespec ts2;
	
	ts1 =  current_kernel_time();
	
	asm volatile(
		".arch_extension sec\n\t"
		"smc #0\n\t");

	ts2 =  current_kernel_time();
	
	printk(KERN_ALERT "ts1.tv_sec:%ld, ts1.tv_nsec:%ld\n", ts1.tv_sec, ts1.tv_nsec);
	printk(KERN_ALERT "ts2.tv_sec:%ld, ts2.tv_nsec:%ld\n", ts2.tv_sec, ts2.tv_nsec);
	printk(KERN_ALERT "Time Difference:%ld\n", ts2.tv_nsec-ts1.tv_nsec);
	
	return 0;
}

static void __exit ktime_exit(void)
{
	printk(KERN_ALERT "exit the module\n");
}

module_init(ktime_init);  
module_exit(ktime_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dongli Zhang");
MODULE_DESCRIPTION("DEV driver for TrustZone");

