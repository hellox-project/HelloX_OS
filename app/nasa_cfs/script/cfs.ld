/* Linker script for the HelloX cFS port.
 * Output format: PE32 executable for i386, suitable for HelloX OS.
 * Sections are arranged compactly so .reloc is right after .data/.bss
 * (avoids the large SizeOfImage that the default layout produces). */

OUTPUT_FORMAT(pei-i386)
OUTPUT_ARCH(i386)

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

    .buildid : ALIGN(4096)
    {
        *(.buildid)
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
