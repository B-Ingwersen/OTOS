OUTPUT_FORMAT("elf32-i386", "elf32-i386",
              "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
phys = 0x00000000;
SECTIONS
{
    .text : {
        code = .;
        *(.text)
        *(.rodata)
        . = ALIGN(4);
    }
    .bss :
    {
        bss = .;
        *(.bss)
        . = ALIGN(4);
    }
    . = . + SIZEOF(.bss);
    .data : 
    {
        data = .;
        *(.data)
        . = ALIGN(4);
    }
    /*.got.plt :
    {
        got.plt = .;
        *(.got.plt)
        . = ALIGN(4);
    }*/
    end = .;
}
