# export CROSS_COMPILE=arm-none-linux-gnueabi-
# export ARCH=arm
# make imx5_defconfig

sudo dd if=arch/arm/boot/uImage of=/dev/mmcblk0 bs=512 seek=2048
