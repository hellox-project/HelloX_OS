/* Linker script for HelloX PE/EXE executable.
 * Output format: PE32 executable for i386, suitable for HelloX OS.
 * This script ensures _hx_main is the entry point and
 * sections are arranged as HelloX expects.
 * No external dependencies. */

OUTPUT_FORMAT(pei-i386)
OUTPUT_ARCH(i386)
ENTRY(_hx_main)

SECTIONS
{
    . = 0x00400000;  /* Default load address for HelloX PE executables */

    .text : ALIGN(4096)
    {
        *(.text)
        *(.text.*)
    }

    .rdata : ALIGN(4096)
    {
        *(.rdata)
        *(.rdata.*)
        *(.rodata)
        *(.rodata.*)
    }

    .data : ALIGN(4096)
    {
        *(.data)
        *(.data.*)
    }

    .bss : ALIGN(4096)
    {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }

    .reloc : ALIGN(4096)
    {
        *(.reloc)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.debug_*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
}
