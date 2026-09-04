SECTIONS
{
    HIDDEN(__romPos = 0);
    header_ROM_START = __romPos;
    header_VRAM = ADDR(.header);
    .header 0x80080000 : AT(header_ROM_START) SUBALIGN(16)
    {
        FILL(0x00000000);
        header_DATA_START = .;
        build/p1-jp/atlus/asm/p1-jp/atlus/header.s.o(.data);
        header_DATA_END = .;
        header_DATA_SIZE = ABSOLUTE(header_DATA_END - header_DATA_START);
    }
    __romPos += SIZEOF(.header);
    header_ROM_END = __romPos;
    header_VRAM_END = .;

    atlus_ROM_START = __romPos;
    atlus_VRAM = ADDR(.atlus);
    .atlus 0x80080000 : AT(atlus_ROM_START) SUBALIGN(16)
    {
        FILL(0x00000000);
        atlus_TEXT_START = .;
        build/p1-jp/atlus/asm/p1-jp/atlus/atlus.s.o(.text);
        . = ALIGN(., 16);
        atlus_TEXT_END = .;
        atlus_TEXT_SIZE = ABSOLUTE(atlus_TEXT_END - atlus_TEXT_START);
        atlus_DATA_START = .;
        build/p1-jp/atlus/asm/p1-jp/atlus/atlus.s.o(.data);
        . = ALIGN(., 16);
        atlus_DATA_END = .;
        atlus_DATA_SIZE = ABSOLUTE(atlus_DATA_END - atlus_DATA_START);
        atlus_RODATA_START = .;
        build/p1-jp/atlus/asm/p1-jp/atlus/atlus.s.o(.rodata);
        . = ALIGN(., 16);
        atlus_RODATA_END = .;
        atlus_RODATA_SIZE = ABSOLUTE(atlus_RODATA_END - atlus_RODATA_START);
        atlus_BSS_START = .;
        build/p1-jp/atlus/asm/p1-jp/atlus/atlus.s.o(.bss);
        . = ALIGN(., 16);
        atlus_BSS_END = .;
        atlus_BSS_SIZE = ABSOLUTE(atlus_BSS_END - atlus_BSS_START);
    }
    __romPos += SIZEOF(.atlus);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    atlus_ROM_END = __romPos;
    atlus_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
