# export CROSS_COMPILE=arm-none-linux-gnueabi-
# export ARCH=arm
# make mx53loco_defconfig

# files changed:
# cpu/arm_cortexa8/config.mk
# board/freescale/mx53_loco/mx53_loco.c dram_init()
# lib_arm/Makefile

sudo dd if=u-boot.imx of=/dev/mmcblk0 bs=512 seek=2
