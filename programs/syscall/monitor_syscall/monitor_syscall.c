#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/syscalls.h>

//ssize_t *sys_call_table = (ssize_t *)0x800329c4;
//ssize_t *sys_call_table = (ssize_t *)NULL;
unsigned long *sys_call_table = (unsigned long *)NULL;

void get_sys_call_table(void)
{
	void *swi_addr = (long *)0xffff0008;
	unsigned long offset = 0;
	unsigned long *vector_swi_addr = 0;

	offset = ((*(long *)swi_addr)&0xfff)+8;
	vector_swi_addr = *(unsigned long *)(swi_addr+offset);

	while(vector_swi_addr++)
	{
		if(((*(unsigned long *)vector_swi_addr)&0xfffff000) == 0xe28f8000)
		{
			offset = ((*(unsigned long *)vector_swi_addr)&0xfff)+8;
			sys_call_table = (void *)vector_swi_addr+offset;
			break;
		}
	}

	printk(KERN_ALERT "sys_call_table base: 0x%08x\n", sys_call_table);

	return;
}

void print_syscall_table(void)
{
	unsigned long sys_num = 0;

	//while(sys_call_table[sys_num] != 0)
	for(sys_num=0; sys_num<10; sys_num++)
	{
		printk(KERN_ALERT "%d: 0x%08x\n", sys_num, sys_call_table[sys_num]);
		//sys_num++;
	}
}

static int __init monitor_init(void)
{
	get_sys_call_table();
	print_syscall_table();
	return 0;
}

static void __exit monitor_exit(void)
{
	printk(KERN_ALERT "Exit from syscall monitor\n");
}

module_init(monitor_init);  
module_exit(monitor_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dongli Zhang");
MODULE_DESCRIPTION("A module to monitor system calls");

