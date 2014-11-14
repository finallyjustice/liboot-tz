#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/syscalls.h>

//ssize_t *sys_call_table = (ssize_t *)0x800329c4;
ssize_t *sys_call_table = (ssize_t *)NULL;

// obtain the base address of system call table 
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

	return;
}

asmlinkage ssize_t (*old_mkdir)(const char __user *pathname, int mode);

asmlinkage ssize_t new_mkdir(const char __user *pathname, int mode)
{
	printk(KERN_ALERT "mkdir path name: %s\n", pathname);
	printk(KERN_ALERT "mkdir not working\n");
	return 0;
}

static int __init hook_init(void)
{
	get_sys_call_table();
	printk(KERN_ALERT "syscall table: 0x%8x\n", sys_call_table);
	old_mkdir = sys_call_table[__NR_mkdir];
	sys_call_table[__NR_mkdir] = new_mkdir;
	printk(KERN_ALERT "hook %d successfully! 0x%08x\n", __NR_mkdir, old_mkdir);
	return 0;
}

static void __exit hook_exit(void)
{
	sys_call_table[__NR_mkdir] = old_mkdir;
	printk(KERN_ALERT "recover system call! 0x%08x\n", sys_call_table[__NR_mkdir]);
}

module_init(hook_init);  
module_exit(hook_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dongli Zhang");
MODULE_DESCRIPTION("A module to hook the system calls");

