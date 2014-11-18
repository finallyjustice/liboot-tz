// This is the program to map physical memory address space in user space

// arm-none-linux-gnueabi-gcc arm-mmap.c -o arm-mmap

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SECURE_DATA_BASE 0xC5100000
#define SECURE_DATA_SIZE 4096

int main(int argc, char **argv)
{
	printf("Hello World!\n");

	int fd;
	unsigned char *map_base;

	// open /dev/mem
	fd = open("/dev/mem", O_RDWR|O_SYNC);
	if(fd < 0)
	{
		printf("Cannot open /dev/mem\n");
		return -1;
	}

	// map the physical memory
	map_base = mmap(NULL, SECURE_DATA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, SECURE_DATA_BASE);
	if(map_base == 0)
	{
		printf("mmap failed\n");
		close(fd);
		return -1;
	}
	else
	{
		printf("mmap successfully\n");
	}

	unsigned long *addr = (unsigned long *)map_base;
	printf("VAL 1: 0x%8x\n", *addr);
	addr++;
	printf("VAL 2: 0x%8x\n", *addr);

	munmap(map_base, SECURE_DATA_SIZE);
	// close /dev/mem
	close(fd);
	

	return 0;
}
