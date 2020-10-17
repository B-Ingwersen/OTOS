OUTPUT_FORMAT("binary")
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
    end = .;
}
