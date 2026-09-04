SECTIONS
{
    HIDDEN(__romPos = 0);
    dng_ROM_START = __romPos;
    dng_VRAM = ADDR(.dng);
    .dng 0x800643A0 : AT(dng_ROM_START) SUBALIGN(4)
    {
        FILL(0x00000000);
        dng_DATA_START = .;
        build/p1-jp/dng/asm/p1-jp/dng/data/dng_id.data.s.o(.data);
        build/p1-jp/dng/asm/p1-jp/dng/dng.s.o(.data);
        dng_DATA_END = .;
        dng_DATA_SIZE = ABSOLUTE(dng_DATA_END - dng_DATA_START);
        dng_TEXT_START = .;
        build/p1-jp/dng/asm/p1-jp/dng/dng.s.o(.text);
        dng_TEXT_END = .;
        dng_TEXT_SIZE = ABSOLUTE(dng_TEXT_END - dng_TEXT_START);
        dng_RODATA_START = .;
        build/p1-jp/dng/asm/p1-jp/dng/dng.s.o(.rodata);
        dng_RODATA_END = .;
        dng_RODATA_SIZE = ABSOLUTE(dng_RODATA_END - dng_RODATA_START);
        dng_BSS_START = .;
        build/p1-jp/dng/asm/p1-jp/dng/dng.s.o(.bss);
        dng_BSS_END = .;
        dng_BSS_SIZE = ABSOLUTE(dng_BSS_END - dng_BSS_START);
    }
    __romPos += SIZEOF(.dng);
    __romPos = ALIGN(__romPos, 16);
    dng_ROM_END = __romPos;
    dng_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
