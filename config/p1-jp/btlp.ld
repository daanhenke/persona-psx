SECTIONS
{
    HIDDEN(__romPos = 0);
    btlp_ROM_START = __romPos;
    btlp_VRAM = ADDR(.btlp);
    .btlp 0x800643A0 : AT(btlp_ROM_START) SUBALIGN(4)
    {
        FILL(0x00000000);
        btlp_DATA_START = .;
        build/p1-jp/btlp/asm/p1-jp/btlp/data/btlp_id.data.s.o(.data);
        build/p1-jp/btlp/asm/p1-jp/btlp/btlp.s.o(.data);
        btlp_DATA_END = .;
        btlp_DATA_SIZE = ABSOLUTE(btlp_DATA_END - btlp_DATA_START);
        btlp_TEXT_START = .;
        build/p1-jp/btlp/asm/p1-jp/btlp/btlp.s.o(.text);
        btlp_TEXT_END = .;
        btlp_TEXT_SIZE = ABSOLUTE(btlp_TEXT_END - btlp_TEXT_START);
        btlp_RODATA_START = .;
        build/p1-jp/btlp/asm/p1-jp/btlp/btlp.s.o(.rodata);
        btlp_RODATA_END = .;
        btlp_RODATA_SIZE = ABSOLUTE(btlp_RODATA_END - btlp_RODATA_START);
        btlp_BSS_START = .;
        build/p1-jp/btlp/asm/p1-jp/btlp/btlp.s.o(.bss);
        btlp_BSS_END = .;
        btlp_BSS_SIZE = ABSOLUTE(btlp_BSS_END - btlp_BSS_START);
    }
    __romPos += SIZEOF(.btlp);
    __romPos = ALIGN(__romPos, 16);
    btlp_ROM_END = __romPos;
    btlp_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
