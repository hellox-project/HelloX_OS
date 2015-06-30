#!/bin/bash

#编译内核

if [ ! -z $1 ]; then
LPWD=`pwd`
cd ../; make clean && make
MAKE_RETVAL=$?
echo "$MAKE_RETVAL"

if [ ! $MAKE_RETVAL -eq 0 ]; then
	echo "编译失败"
	exit
fi
cd $LPWD

fi

echo "制作软盘中。。。"

KERNEL_ELF=hellox_kernel
HELLOX_IMG=hellox.img
BOOT_BIN=bootsect.bin  
REALINIT_BIN=realinit.bin
MINIKER_BIN=miniker.bin


KERNEL_BIN=kernel.bin
#KERNEL_BIN=master.bin

##拷贝上页输出内核
cp ../$KERNEL_ELF .

#将内核elf文件中二进制提取到bin
objcopy -O binary -j .rodata -j .text -j .data -j .bss  -S -g $KERNEL_ELF $KERNEL_BIN
echo "$KERNEL_BIN"

echo "生成空白软盘镜像文件"
dd if=/dev/zero of=$HELLOX_IMG bs=512 count=2880 

echo "写入$BOOT_BIN"
dd if=$BOOT_BIN of=$HELLOX_IMG bs=512 count=1 conv=notrunc

echo "写入$REALINIT_BIN"
dd if=$REALINIT_BIN of=$HELLOX_IMG bs=512 seek=2 conv=notrunc

echo "写入$MINIKER_BIN"
dd if=$MINIKER_BIN of=$HELLOX_IMG bs=512 seek=10 conv=notrunc

echo "写入$KERNEL_BIN"
dd if=$KERNEL_BIN of=$HELLOX_IMG  bs=512 seek=264 conv=notrunc

echo "写入完毕:$HELLOX_IMG"
