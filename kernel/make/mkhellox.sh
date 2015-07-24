#!/bin/bash

KERNEL_ELF=kernel.elf
HELLOX_IMG=hellox.img
BOOT_BIN=bootsect.bin  
REALINIT_BIN=realinit.bin
MINIKER_BIN=miniker.bin

KERNEL_BIN=master.bin
MASTER_BIN=$KERNEL_BIN

##编译内核
make_kernel(){
    cd ..
    make
}

clean_kernel(){
    cd ..
    make clean
}

##制作内核bin文件
make_kernel_bin(){
    ##拷贝上页输出内核
    KERNEL_ELF_FILE=$KERNEL_ELF
    test -f  $KERNEL_ELF_FILE || (file -b $KERNEL_ELF_FILE; exit 1) 
    
    #将内核elf文件中二进制提取到bin
    #objcopy -O binary -j .rodata -j .text -j .data -j .bss  -S -g $KERNEL_ELF $KERNEL_BIN
    objcopy -v -O binary -j .rodata -j .text -j .data -j .bss $KERNEL_ELF_FILE $KERNEL_BIN
}

main(){
if [ -z $1 ]; then
    echo "Usage $0  make     编译内核代码
                    clean    清理内核代码"
    exit 1;
fi

    LPWD=`pwd`

    if [ $1 == "make" ]; then
        ## 编译内核
        make_kernel
        MAKE_RETVAL=$?
        echo "make retval:$MAKE_RETVAL"
        if [ ! $MAKE_RETVAL -eq 0 ]; then
	        echo "编译失败"
	        exit 1
        fi

        ## 输出bin文件
        make_kernel_bin
        MAKE_RETVAL=$?

        if [ ! $MAKE_RETVAL -eq 0 ]; then
            echo "制作bin文件失败"
        fi
    fi
   
    if [ $1 == "clean" ]; then
        clean_kernel
        return 1;
    fi
    cd $LPWD
}

main $@
RETVAL=$?
if [ ! $RETVAL -eq 0 ]; then
    echo "出现错误"
    exit $RETVAL;
fi

cp -v ../$KERNEL_BIN ../../tools/vfmaker/$MASTER_BIN

exit 0

rm -rf $HELLOX_IMG

echo "生成空白软盘镜像文件"
dd if=/dev/zero of=$HELLOX_IMG bs=512 count=2880 

echo "写入$BOOT_BIN"
dd if=$BOOT_BIN of=$HELLOX_IMG bs=512 count=1 conv=notrunc

echo "写入$REALINIT_BIN"
dd if=$REALINIT_BIN of=$HELLOX_IMG bs=512 seek=2 conv=notrunc

echo "写入$MINIKER_BIN"
dd if=$MINIKER_BIN of=$HELLOX_IMG bs=512 seek=10 conv=notrunc

echo "写入$KERNEL_BIN"
dd if=$KERNEL_BIN of=$HELLOX_IMG  bs=512 seek=264 count=6 conv=notrunc
dd if=$KERNEL_BIN of=$HELLOX_IMG  bs=512 skip=6 seek=288 count=300 conv=notrunc
dd if=$KERNEL_BIN of=$HELLOX_IMG  bs=512 skip=306 seek=302 conv=notrunc


echo "写入完毕:$HELLOX_IMG"
