/*
 * Compiler-runtime shim for the freestanding MinGW (i686-w64-windows-gnu)
 * build. clang emits a call to __main() at the start of the application's
 * main() to run C++ static initializers. In a freestanding HelloX build there
 * is no libmingw32 to provide it, so we supply a no-op definition.
 * (Analogous to comp_gcc_AI.c, which supplies the 64-bit division helpers.)
 */
void __main(void)
{
}
