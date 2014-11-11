#include <common.h>
#include <command.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <asm/byteorder.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/bootm.h>
#include <asm/secure.h>
#include <linux/compiler.h>

#define R32   (volatile unsigned long *)
#define R16   (volatile unsigned short *)
#define R8    (volatile unsigned char *)
#define MY_CSU_BASE_ADDR   0x63F9C000

enum
{
	CSL0    = MY_CSU_BASE_ADDR + 0x00,
	CSL1    = MY_CSU_BASE_ADDR + 0x04,
	CSL2    = MY_CSU_BASE_ADDR + 0x08,         // GPIO7
	CSL3    = MY_CSU_BASE_ADDR + 0x0c,
	CSL4    = MY_CSU_BASE_ADDR + 0x10,
	CSL5    = MY_CSU_BASE_ADDR + 0x14,
	CSL6    = MY_CSU_BASE_ADDR + 0x18,
	CSL7    = MY_CSU_BASE_ADDR + 0x1c,
	CSL8    = MY_CSU_BASE_ADDR + 0x20,
	CSL9    = MY_CSU_BASE_ADDR + 0x24,
	CSL10   = MY_CSU_BASE_ADDR + 0x28,
	CSL11   = MY_CSU_BASE_ADDR + 0x2c,
	CSL12   = MY_CSU_BASE_ADDR + 0x30,         // GPIO7
	CSL13   = MY_CSU_BASE_ADDR + 0x34,
	CSL14   = MY_CSU_BASE_ADDR + 0x38,
	CSL15   = MY_CSU_BASE_ADDR + 0x3c,
	CSL16   = MY_CSU_BASE_ADDR + 0x40,
	CSL17   = MY_CSU_BASE_ADDR + 0x44,
	CSL18   = MY_CSU_BASE_ADDR + 0x48,
	CSL19   = MY_CSU_BASE_ADDR + 0x4c,
	CSL20   = MY_CSU_BASE_ADDR + 0x50,
	CSL21   = MY_CSU_BASE_ADDR + 0x54,
	CSL22   = MY_CSU_BASE_ADDR + 0x58,
	CSL23   = MY_CSU_BASE_ADDR + 0x5c,
	CSL24   = MY_CSU_BASE_ADDR + 0x60,
	CSL25   = MY_CSU_BASE_ADDR + 0x64,         // ESDHC
	CSL26   = MY_CSU_BASE_ADDR + 0x68,
	CSL27   = MY_CSU_BASE_ADDR + 0x6c,
	CSL28   = MY_CSU_BASE_ADDR + 0x70,         // ESDHC
	CSL29   = MY_CSU_BASE_ADDR + 0x74,
	CSL30   = MY_CSU_BASE_ADDR + 0x78,
	CSL31   = MY_CSU_BASE_ADDR + 0x7c,
};

enum
{
	TZIC_INTCTRL = 0x0fffc000,
	TZIC_INTSEC0 = 0x0fffc080,
	TZIC_INTSEC1 = 0x0fffc084,
	TZIC_INTSEC2 = 0x0fffc088,
	TZIC_INTSEC3 = 0x0fffc08c,
	IIM_SCS1 = IMX_IIM_BASE+0x030,
};

enum
{
	TZIC_PRIORITY0  = 0x0fffc400,
	TZIC_PRIORITY1  = 0x0fffc404,
	TZIC_PRIORITY2  = 0x0fffc408,
	TZIC_PRIORITY3  = 0x0fffc40c,
	TZIC_PRIORITY4  = 0x0fffc410,
	TZIC_PRIORITY5  = 0x0fffc414,
	TZIC_PRIORITY6  = 0x0fffc418,
	TZIC_PRIORITY7  = 0x0fffc41c,
	TZIC_PRIORITY8  = 0x0fffc420,
	TZIC_PRIORITY9  = 0x0fffc424,
	TZIC_PRIORITY10 = 0x0fffc428,
	TZIC_PRIORITY11 = 0x0fffc42c,
	TZIC_PRIORITY12 = 0x0fffc430,
	TZIC_PRIORITY13 = 0x0fffc434,
	TZIC_PRIORITY14 = 0x0fffc438,
	TZIC_PRIORITY15 = 0x0fffc43c,
	TZIC_PRIORITY16 = 0x0fffc440,
	TZIC_PRIORITY17 = 0x0fffc444,
	TZIC_PRIORITY18 = 0x0fffc448,
	TZIC_PRIORITY19 = 0x0fffc44c,
	TZIC_PRIORITY20 = 0x0fffc450,
	TZIC_PRIORITY21 = 0x0fffc454,
	TZIC_PRIORITY22 = 0x0fffc458,
	TZIC_PRIORITY23 = 0x0fffc45c,
	TZIC_PRIORITY24 = 0x0fffc460,
	TZIC_PRIORITY25 = 0x0fffc464,
	TZIC_PRIORITY26 = 0x0fffc468,
	TZIC_PRIORITY27 = 0x0fffc46c,
	TZIC_PRIORITY28 = 0x0fffc470,
	TZIC_PRIORITY29 = 0x0fffc474,
	TZIC_PRIORITY30 = 0x0fffc478,
	TZIC_PRIORITY31 = 0x0fffc47c,
};

bootm_headers_t *my_images;
unsigned long my_machid;
unsigned long my_r2;

void boot_linux_kernel_entry(void)
{
	void (*kernel_entry)(int zero, int arch, uint params);
	kernel_entry = (void (*)(int, int, uint))my_images->ep;
	kernel_entry(0, my_machid, my_r2);
}

extern void init_secure_monitor();

void start_transition(bootm_headers_t *images, unsigned long machid, unsigned long r2)
{
	*R32 CSL0  = 0x00ff00ff;
	*R32 CSL1  = 0x00ff00ff;
	*R32 CSL2  = 0x00ff00ff;
	*R32 CSL3  = 0x00ff00ff;
	*R32 CSL4  = 0x00ff00ff;
	*R32 CSL5  = 0x00ff00ff;
	*R32 CSL6  = 0x00ff00ff;
	*R32 CSL7  = 0x00ff00ff;
	*R32 CSL8  = 0x00ff00ff;
	*R32 CSL9  = 0x00ff00ff;
	*R32 CSL10 = 0x00ff00ff;
	*R32 CSL11 = 0x00ff00ff;
	*R32 CSL12 = 0x00ff00ff;
	*R32 CSL13 = 0x00ff00ff;
	*R32 CSL14 = 0x00ff00ff;
	*R32 CSL15 = 0x00ff00ff;
	*R32 CSL16 = 0x00ff00ff;
	*R32 CSL17 = 0x00ff00ff;
	*R32 CSL18 = 0x00ff00ff;
	*R32 CSL19 = 0x00ff00ff;
	*R32 CSL20 = 0x00ff00ff;
	*R32 CSL21 = 0x00ff00ff;
	*R32 CSL22 = 0x00ff00ff;
	*R32 CSL23 = 0x00ff00ff;
	*R32 CSL24 = 0x00ff00ff;
	*R32 CSL25 = 0x00ff00ff;
	*R32 CSL26 = 0x00ff00ff;
	*R32 CSL27 = 0x00ff00ff;
	*R32 CSL28 = 0x00ff00ff;
	*R32 CSL29 = 0x00ff00ff;
	*R32 CSL30 = 0x00ff00ff;
	*R32 CSL31 = 0x00ff00ff;

	*R32 TZIC_INTSEC0 = 0xffffffff;
	*R32 TZIC_INTSEC1 = 0xffffffff;
	*R32 TZIC_INTSEC2 = 0xffffffff;
	*R32 TZIC_INTSEC3 = 0xffffffff;

	*R32 TZIC_PRIORITY0  = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY1  = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY2  = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY3  = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY4  = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY5  = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY6  = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY7  = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY8  = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY9  = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY10 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY11 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY12 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY13 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY14 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY15 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY16 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY17 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY18 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY19 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY20 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY21 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY22 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY23 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY24 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY25 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY26 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY27 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY28 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY29 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY30 = 0x1f1f1f1f;
	*R32 TZIC_PRIORITY31 = 0x1f1f1f1f;

	int scr_val = 1;

	__asm__ volatile(
			"mrc p15, 0, r0, c1, c1, 0\n\t"
			"mov %0, r0\n\t"
			: "=r"(scr_val)
			:
			: "r0"
	);

	if(scr_val & 0x1)
		printf("TrustZone NS Bit is 1 - Normal\n");
	else
		printf("TrustZone NS Bit is 0 - Secure\n");

	printf("SCR Reg: 0x%x\n", scr_val);

	printf("Begin Set TrustZone NS Bit to 1\n");

	unsigned int intctrl = *R32 TZIC_INTCTRL;
	printf("TZIC_INTCTRL is : 0x%x\n", intctrl);

	intctrl = intctrl | 0x10001;
	//intctrl = intctrl | 0x80010001;
	*R32 TZIC_INTCTRL = intctrl;

	unsigned char scs1 = *R8 IIM_SCS1;
	scs1 = scs1 | 0x7d;
	*R8 IIM_SCS1 = scs1;

	printf("Now TZIC_INTCTRL is : 0x%x\n", intctrl);

	__asm__ volatile(
			"mrc p15, 0, r0, c1, c1, 0\n\t"
			"mov r1, r0\n\t"
			"orr r0, r1, #0x31\n\t"
			"mcr p15, 0, r0, c1, c1, 0\n\t"
			:
			:
			: "r0","r1"
	);

	printf("End Set TrustZone NS Bit to 1\n");
	printf("Jump to Linux in Normal World! \n");

	my_images = images;
	my_machid = machid;
	my_r2     = r2;

	boot_linux_kernel_entry();

	return;
}
