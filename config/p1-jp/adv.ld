SECTIONS
{
    HIDDEN(__romPos = 0);
    adv_ROM_START = __romPos;
    adv_VRAM = ADDR(.adv);
    .adv 0x800643A0 : AT(adv_ROM_START) SUBALIGN(4)
    {
        FILL(0x00000000);
        adv_DATA_START = .;
        build/p1-jp/adv/asm/p1-jp/adv/data/adv_id.data.s.o(.data);
        build/p1-jp/adv/asm/p1-jp/adv/adv.s.o(.data);
        adv_DATA_END = .;
        adv_DATA_SIZE = ABSOLUTE(adv_DATA_END - adv_DATA_START);
        adv_TEXT_START = .;
        build/p1-jp/adv/asm/p1-jp/adv/adv.s.o(.text);
        adv_TEXT_END = .;
        adv_TEXT_SIZE = ABSOLUTE(adv_TEXT_END - adv_TEXT_START);
        adv_RODATA_START = .;
        build/p1-jp/adv/asm/p1-jp/adv/adv.s.o(.rodata);
        adv_RODATA_END = .;
        adv_RODATA_SIZE = ABSOLUTE(adv_RODATA_END - adv_RODATA_START);
        adv_BSS_START = .;
        build/p1-jp/adv/asm/p1-jp/adv/adv.s.o(.bss);
        adv_BSS_END = .;
        adv_BSS_SIZE = ABSOLUTE(adv_BSS_END - adv_BSS_START);
    }
    __romPos += SIZEOF(.adv);
    __romPos = ALIGN(__romPos, 16);
    adv_ROM_END = __romPos;
    adv_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
