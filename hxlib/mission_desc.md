Main issues:
1. No relocation table, should be processed properly in UserAgent;
2. HelloX.c files, __clang__ ASM code, may exist bugs, should review carefully, by AI or manually; ASM code's return value should be checked, since __InterlockedXXX's return path is not same between MSC_VER and clang;
3. Makefile should apply: no-pic, include reloc table, and ...
4. 

1. Use LIB directory to refer the current directory(~/hellox/v190_lib);
2. Read all source files in LIB directory and it's subdirectory, understand the source code;
3. Write a makefile in LIB directory, to compile and link all source files in this directory and it's sub-directory, comply the following rules:
3.1 Use clang as compiler;
3.2 Output format is PE/EXE, which could be loaded by HelloX;
3.3 The entry point is _hx_main;
3.4 Do not depend any external library or header files, since HelloX can not support;

Just write a makefile, NO NEED compile it, then STOP to review;

