# export CROSS_COMPILE=arm-none-linux-gnueabi-
# export ARCH=arm
# make mx53loco_defconfig

sudo dd if=u-boot.imx of=/dev/mmcblk0 bs=512 seek=2
