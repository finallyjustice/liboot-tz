#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/cdev.h>

#include <asm/current.h>
#include <asm/segment.h>
#include <asm/uaccess.h>

struct cdev cdev;
int dev_major = 50;
int dev_minor = 0;

int go_secure_world(void)
{
	printk(KERN_ALERT "Transiting to TrustZone Secure World...\n");

	asm volatile(
		".arch_extension sec\n\t"
		"smc #0\n\t");

	printk(KERN_ALERT "Back to TrustZone Normal World Linux.\n");

	return 0;
}

int tz_smc_open(struct inode *inode, struct file *filep)
{
	//printk(KERN_ALERT "tz_smc_open\n");
	return 0;
}

int tz_smc_release(struct inode *inode, struct file *filep)
{
	//printk(KERN_ALERT "tz_smc_release\n");
	return 0;
}

ssize_t tz_smc_read(struct file *filep, char __user *buf, size_t count, loff_t *offp)
{
	//char *my_data = (char *)vmalloc(100);
	//memset(my_data, 0, 100);
	//sprintf(my_data, "%s", "Hello World");
	//if(copy_to_user(buf, my_data, strlen(my_data)) != 0)
	//	printk(KERN_ALERT "copy_to_user failed\n");
//####printk(KERN_ALERT "tz_smc_read\n");
	//vfree(my_data);
	//return strlen(my_data);
	go_secure_world();
	return 0;
}

ssize_t tz_smc_write(struct file *filep, const char __user *buf, size_t count, loff_t *offp)
{
	printk(KERN_ALERT "tz_smc_write\n");
	return 0;
}

long tz_smc_ioctl(struct file *filp, unsigned int ioctl, unsigned long arg)
{
	return 0;
}

struct file_operations tz_smc_fops = {
	owner:   THIS_MODULE, 
	open:    tz_smc_open,    
	read:    tz_smc_read,
	write:   tz_smc_write,
	release: tz_smc_release,
	unlocked_ioctl : tz_smc_ioctl,
};

int cdev_tz_smc_install(void)
{
	int ret;
	dev_t devno = MKDEV(dev_major, dev_minor);

	if(dev_major)  // assign static dev number
	{
		ret = register_chrdev_region(devno, 2, "tz_smc");
	}
	else  // assign dynamic dev number
	{
		ret = alloc_chrdev_region(&devno, 0, 2, "tz_smc");
		dev_major = MAJOR(devno);
	}

	if(ret < 0)
	{
		printk(KERN_ALERT "/dev/tz_smc register failed\n");
		return ret;
	}
	else
	{
		printk(KERN_ALERT "/dev/tz_smc register successful\n");
	}

	// init the cdev
	cdev_init(&cdev, &tz_smc_fops);
	cdev.owner = THIS_MODULE;
	cdev.ops = &tz_smc_fops;
	// register the char device
	cdev_add(&cdev, MKDEV(dev_major, 0), 1);

	return 0;
}

void cdev_tz_smc_uninstall(void)
{
	cdev_del(&cdev);
	unregister_chrdev_region(MKDEV(dev_major, 0), 1);
}

static int __init dev_smc_init(void)
{
	int ret;
	printk(KERN_ALERT "init the module\n");
	ret = cdev_tz_smc_install();
	return ret;
}

static void __exit dev_smc_exit(void)
{
	printk(KERN_ALERT "exit the module\n");
	cdev_tz_smc_uninstall();
}

module_init(dev_smc_init);  
module_exit(dev_smc_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dongli Zhang");
MODULE_DESCRIPTION("DEV driver for TrustZone");

