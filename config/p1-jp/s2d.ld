SECTIONS
{
    HIDDEN(__romPos = 0);
    s2d_ROM_START = __romPos;
    s2d_VRAM = ADDR(.s2d);
    .s2d 0x800643A0 : AT(s2d_ROM_START) SUBALIGN(4)
    {
        FILL(0x00000000);
        s2d_DATA_START = .;
        build/p1-jp/s2d/asm/p1-jp/s2d/data/s2d_id.data.s.o(.data);
        build/p1-jp/s2d/asm/p1-jp/s2d/s2d.s.o(.data);
        s2d_DATA_END = .;
        s2d_DATA_SIZE = ABSOLUTE(s2d_DATA_END - s2d_DATA_START);
        s2d_TEXT_START = .;
        build/p1-jp/s2d/asm/p1-jp/s2d/s2d.s.o(.text);
        s2d_TEXT_END = .;
        s2d_TEXT_SIZE = ABSOLUTE(s2d_TEXT_END - s2d_TEXT_START);
        s2d_RODATA_START = .;
        build/p1-jp/s2d/asm/p1-jp/s2d/s2d.s.o(.rodata);
        s2d_RODATA_END = .;
        s2d_RODATA_SIZE = ABSOLUTE(s2d_RODATA_END - s2d_RODATA_START);
        s2d_BSS_START = .;
        build/p1-jp/s2d/asm/p1-jp/s2d/s2d.s.o(.bss);
        s2d_BSS_END = .;
        s2d_BSS_SIZE = ABSOLUTE(s2d_BSS_END - s2d_BSS_START);
    }
    __romPos += SIZEOF(.s2d);
    __romPos = ALIGN(__romPos, 16);
    s2d_ROM_END = __romPos;
    s2d_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
