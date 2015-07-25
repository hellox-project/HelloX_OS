# GCC编译环境介绍

## 实现GCC编译模块
	目前已经实现kernel使用GCC编译。

## 构建系统
	automake

## 配置说明
### HelloX_OS/kernel/kernel.mk
	Automake使用的编译选项，内核编译各个模块Makefile.am都include该文件， 如果模块需要自定义参数，则可以修改模块下的Makefile.am。
	
### HelloX_OS/kernel/make/mkhellox.sh
	编译脚本文件。


###编译内核
1. cd HelloX_OS 
	
	./amake.sh
2. cd kernel
	
	./mkhellox.sh make
	负责编译，生成的master.bin， 拷贝master.bin到HelloX_OS/tools/vfmaker

###清理
	./mkhellox.sh clean