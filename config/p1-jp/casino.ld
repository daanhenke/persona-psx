SECTIONS
{
    HIDDEN(__romPos = 0);
    casino_ROM_START = __romPos;
    casino_VRAM = ADDR(.casino);
    .casino 0x800643A0 : AT(casino_ROM_START) SUBALIGN(4)
    {
        FILL(0x00000000);
        casino_DATA_START = .;
        build/p1-jp/casino/asm/p1-jp/casino/data/casino_id.data.s.o(.data);
        build/p1-jp/casino/asm/p1-jp/casino/casino.s.o(.data);
        casino_DATA_END = .;
        casino_DATA_SIZE = ABSOLUTE(casino_DATA_END - casino_DATA_START);
        casino_TEXT_START = .;
        build/p1-jp/casino/asm/p1-jp/casino/casino.s.o(.text);
        casino_TEXT_END = .;
        casino_TEXT_SIZE = ABSOLUTE(casino_TEXT_END - casino_TEXT_START);
        casino_RODATA_START = .;
        build/p1-jp/casino/asm/p1-jp/casino/casino.s.o(.rodata);
        casino_RODATA_END = .;
        casino_RODATA_SIZE = ABSOLUTE(casino_RODATA_END - casino_RODATA_START);
        casino_BSS_START = .;
        build/p1-jp/casino/asm/p1-jp/casino/casino.s.o(.bss);
        casino_BSS_END = .;
        casino_BSS_SIZE = ABSOLUTE(casino_BSS_END - casino_BSS_START);
    }
    __romPos += SIZEOF(.casino);
    __romPos = ALIGN(__romPos, 16);
    casino_ROM_END = __romPos;
    casino_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
