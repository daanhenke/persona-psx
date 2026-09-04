SECTIONS
{
    HIDDEN(__romPos = 0);
    header_ROM_START = __romPos;
    header_VRAM = ADDR(.header);
    .header 0x80080000 : AT(header_ROM_START) SUBALIGN(16)
    {
        FILL(0x00000000);
        header_DATA_START = .;
        build/p1-jp/open/asm/p1-jp/open/header.s.o(.data);
        header_DATA_END = .;
        header_DATA_SIZE = ABSOLUTE(header_DATA_END - header_DATA_START);
    }
    __romPos += SIZEOF(.header);
    header_ROM_END = __romPos;
    header_VRAM_END = .;

    open_ROM_START = __romPos;
    open_VRAM = ADDR(.open);
    .open 0x80080000 : AT(open_ROM_START) SUBALIGN(16)
    {
        FILL(0x00000000);
        open_TEXT_START = .;
        build/p1-jp/open/asm/p1-jp/open/open.s.o(.text);
        . = ALIGN(., 16);
        open_TEXT_END = .;
        open_TEXT_SIZE = ABSOLUTE(open_TEXT_END - open_TEXT_START);
        open_DATA_START = .;
        build/p1-jp/open/asm/p1-jp/open/open.s.o(.data);
        . = ALIGN(., 16);
        open_DATA_END = .;
        open_DATA_SIZE = ABSOLUTE(open_DATA_END - open_DATA_START);
        open_RODATA_START = .;
        build/p1-jp/open/asm/p1-jp/open/open.s.o(.rodata);
        . = ALIGN(., 16);
        open_RODATA_END = .;
        open_RODATA_SIZE = ABSOLUTE(open_RODATA_END - open_RODATA_START);
        open_BSS_START = .;
        build/p1-jp/open/asm/p1-jp/open/open.s.o(.bss);
        . = ALIGN(., 16);
        open_BSS_END = .;
        open_BSS_SIZE = ABSOLUTE(open_BSS_END - open_BSS_START);
    }
    __romPos += SIZEOF(.open);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    open_ROM_END = __romPos;
    open_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
