#!/bin/bash

arch=$1
CONF_OPTS=

if [ -z $arch ]; then
    arch=x86
fi

case $arch in 
    stm32)
    	CONF_OPTS="cflags=-m --host=arm-none-eabi hx_arch=arch/stm32 hx_drivers=drivers/stm32";;
    x86)
    	CONF_OPTS="cflags=-m32";;
esac

echo $CONF_OPTS

autoreconf && automake && ./configure $CONF_OPTS;
