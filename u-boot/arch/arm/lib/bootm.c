/* Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 *  - Added prep subcommand support
 *  - Reorganized source - modeled after powerpc version
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

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

#if defined(CONFIG_ARMV7_NONSEC) || defined(CONFIG_ARMV7_VIRT)
#include <asm/armv7.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static struct tag *params;

static ulong get_sp(void)
{
	ulong ret;

	asm("mov %0, sp" : "=r"(ret) : );
	return ret;
}

void arch_lmb_reserve(struct lmb *lmb)
{
	ulong sp;

	/*
	 * Booting a (Linux) kernel image
	 *
	 * Allocate space for command line and board info - the
	 * address should be as high as possible within the reach of
	 * the kernel (see CONFIG_SYS_BOOTMAPSZ settings), but in unused
	 * memory, which means far enough below the current stack
	 * pointer.
	 */
	sp = get_sp();
	debug("## Current stack ends at 0x%08lx ", sp);

	/* adjust sp by 4K to be safe */
	sp -= 4096;
	lmb_reserve(lmb, sp,
		    gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size - sp);
}

/**
 * announce_and_cleanup() - Print message and prepare for kernel boot
 *
 * @fake: non-zero to do everything except actually boot
 */
static void announce_and_cleanup(int fake)
{
	printf("\nStarting kernel ...%s\n\n", fake ?
		"(fake run for tracing)" : "");
	bootstage_mark_name(BOOTSTAGE_ID_BOOTM_HANDOFF, "start_kernel");
#ifdef CONFIG_BOOTSTAGE_FDT
	bootstage_fdt_add_report();
#endif
#ifdef CONFIG_BOOTSTAGE_REPORT
	bootstage_report();
#endif

#ifdef CONFIG_USB_DEVICE
	udc_disconnect();
#endif
	cleanup_before_linux();
}

static void setup_start_tag (bd_t *bd)
{
	params = (struct tag *)bd->bi_boot_params;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);
}

static void setup_memory_tags(bd_t *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);

		params->u.mem.start = bd->bi_dram[i].start;
		params->u.mem.size = bd->bi_dram[i].size;

		params = tag_next (params);
	}
}

static void setup_commandline_tag(bd_t *bd, char *commandline)
{
	char *p;

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++);

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;

	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;

	strcpy (params->u.cmdline.cmdline, p);

	params = tag_next (params);
}

static void setup_initrd_tag(bd_t *bd, ulong initrd_start, ulong initrd_end)
{
	/* an ATAG_INITRD node tells the kernel where the compressed
	 * ramdisk can be found. ATAG_RDIMG is a better name, actually.
	 */
	params->hdr.tag = ATAG_INITRD2;
	params->hdr.size = tag_size (tag_initrd);

	params->u.initrd.start = initrd_start;
	params->u.initrd.size = initrd_end - initrd_start;

	params = tag_next (params);
}

static void setup_serial_tag(struct tag **tmp)
{
	struct tag *params = *tmp;
	struct tag_serialnr serialnr;

	get_board_serial(&serialnr);
	params->hdr.tag = ATAG_SERIAL;
	params->hdr.size = tag_size (tag_serialnr);
	params->u.serialnr.low = serialnr.low;
	params->u.serialnr.high= serialnr.high;
	params = tag_next (params);
	*tmp = params;
}

static void setup_revision_tag(struct tag **in_params)
{
	u32 rev = 0;

	rev = get_board_rev();
	params->hdr.tag = ATAG_REVISION;
	params->hdr.size = tag_size (tag_revision);
	params->u.revision.rev = rev;
	params = tag_next (params);
}

static void setup_end_tag(bd_t *bd)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

__weak void setup_board_tags(struct tag **in_params) {}

#ifdef CONFIG_ARM64
static void do_nonsec_virt_switch(void)
{
	smp_kick_all_cpus();
	flush_dcache_all();	/* flush cache before swtiching to EL2 */
	armv8_switch_to_el2();
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	armv8_switch_to_el1();
#endif
}
#endif

/* Subcommand: PREP */
static void boot_prep_linux(bootm_headers_t *images)
{
	char *commandline = getenv("bootargs");

	if (IMAGE_ENABLE_OF_LIBFDT && images->ft_len) {
#ifdef CONFIG_OF_LIBFDT
		debug("using: FDT\n");
		if (image_setup_linux(images)) {
			printf("FDT creation failed! hanging...");
			hang();
		}
#endif
	} else if (BOOTM_ENABLE_TAGS) {
		debug("using: ATAGS\n");
		setup_start_tag(gd->bd);
		if (BOOTM_ENABLE_SERIAL_TAG)
			setup_serial_tag(&params);
		if (BOOTM_ENABLE_CMDLINE_TAG)
			setup_commandline_tag(gd->bd, commandline);
		if (BOOTM_ENABLE_REVISION_TAG)
			setup_revision_tag(&params);
		if (BOOTM_ENABLE_MEMORY_TAGS)
			setup_memory_tags(gd->bd);
		if (BOOTM_ENABLE_INITRD_TAG) {
			if (images->rd_start && images->rd_end) {
				setup_initrd_tag(gd->bd, images->rd_start,
						 images->rd_end);
			}
		}
		setup_board_tags(&params);
		setup_end_tag(gd->bd);
	} else {
		printf("FDT and ATAGS support not compiled in - hanging\n");
		hang();
	}
}

// Dongli-Start
// Prepare to switch to normal world
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

void prepare_transition(void)
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
	printf("Jump to Linux in Normal World!\n");

	return;
}

// Dongli-End

/* Subcommand: GO */
static void boot_jump_linux(bootm_headers_t *images, int flag)
{
#ifdef CONFIG_ARM64
	void (*kernel_entry)(void *fdt_addr, void *res0, void *res1,
			void *res2);
	int fake = (flag & BOOTM_STATE_OS_FAKE_GO);

	kernel_entry = (void (*)(void *fdt_addr, void *res0, void *res1,
				void *res2))images->ep;

	debug("## Transferring control to Linux (at address %lx)...\n",
		(ulong) kernel_entry);
	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	announce_and_cleanup(fake);

	if (!fake) {
		do_nonsec_virt_switch();
		kernel_entry(images->ft_addr, NULL, NULL, NULL);
	}
#else
	unsigned long machid = gd->bd->bi_arch_number;
	char *s;
	void (*kernel_entry)(int zero, int arch, uint params);
	unsigned long r2;
	int fake = (flag & BOOTM_STATE_OS_FAKE_GO);

	kernel_entry = (void (*)(int, int, uint))images->ep;

	s = getenv("machid");
	if (s) {
		strict_strtoul(s, 16, &machid);
		printf("Using machid 0x%lx from environment\n", machid);
	}

	debug("## Transferring control to Linux (at address %08lx)" \
		"...\n", (ulong) kernel_entry);
	bootstage_mark(BOOTSTAGE_ID_RUN_OS);
	announce_and_cleanup(fake);

	if (IMAGE_ENABLE_OF_LIBFDT && images->ft_len)
		r2 = (unsigned long)images->ft_addr;
	else
		r2 = gd->bd->bi_boot_params;

	if (!fake) {
#if defined(CONFIG_ARMV7_NONSEC) || defined(CONFIG_ARMV7_VIRT)
		armv7_init_nonsec();
		secure_ram_addr(_do_nonsec_entry)(kernel_entry,
						  0, machid, r2);
#else
		// Dongli-Start
		printf("###### We are jumping to the kernel now!!!!\n");
		prepare_transition();
		// Dongli-End
		kernel_entry(0, machid, r2);
#endif
	}
#endif
}

/* Main Entry point for arm bootm implementation
 *
 * Modeled after the powerpc implementation
 * DIFFERENCE: Instead of calling prep and go at the end
 * they are called if subcommand is equal 0.
 */
int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	/* No need for those on ARM */
	if (flag & BOOTM_STATE_OS_BD_T || flag & BOOTM_STATE_OS_CMDLINE)
		return -1;

	if (flag & BOOTM_STATE_OS_PREP) {
		boot_prep_linux(images);
		return 0;
	}

	if (flag & (BOOTM_STATE_OS_GO | BOOTM_STATE_OS_FAKE_GO)) {
		boot_jump_linux(images, flag);
		return 0;
	}

	boot_prep_linux(images);
	boot_jump_linux(images, flag);
	return 0;
}

#ifdef CONFIG_CMD_BOOTZ

struct zimage_header {
	uint32_t	code[9];
	uint32_t	zi_magic;
	uint32_t	zi_start;
	uint32_t	zi_end;
};

#define	LINUX_ARM_ZIMAGE_MAGIC	0x016f2818

int bootz_setup(ulong image, ulong *start, ulong *end)
{
	struct zimage_header *zi;

	zi = (struct zimage_header *)map_sysmem(image, 0);
	if (zi->zi_magic != LINUX_ARM_ZIMAGE_MAGIC) {
		puts("Bad Linux ARM zImage magic!\n");
		return 1;
	}

	*start = zi->zi_start;
	*end = zi->zi_end;

	printf("Kernel image @ %#08lx [ %#08lx - %#08lx ]\n", image, *start,
	      *end);

	return 0;
}

#endif	/* CONFIG_CMD_BOOTZ */

#if defined(CONFIG_BOOTM_VXWORKS)
void boot_prep_vxworks(bootm_headers_t *images)
{
#if defined(CONFIG_OF_LIBFDT)
	int off;

	if (images->ft_addr) {
		off = fdt_path_offset(images->ft_addr, "/memory");
		if (off < 0) {
			if (arch_fixup_fdt(images->ft_addr))
				puts("## WARNING: fixup memory failed!\n");
		}
	}
#endif
	cleanup_before_linux();
}
void boot_jump_vxworks(bootm_headers_t *images)
{
	/* ARM VxWorks requires device tree physical address to be passed */
	((void (*)(void *))images->ep)(images->ft_addr);
}
#endif
