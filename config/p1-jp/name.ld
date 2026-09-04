SECTIONS
{
    HIDDEN(__romPos = 0);
    name_ROM_START = __romPos;
    name_VRAM = ADDR(.name);
    .name 0x800643A0 : AT(name_ROM_START) SUBALIGN(4)
    {
        FILL(0x00000000);
        name_DATA_START = .;
        build/p1-jp/name/asm/p1-jp/name/data/name_id.data.s.o(.data);
        build/p1-jp/name/asm/p1-jp/name/name.s.o(.data);
        name_DATA_END = .;
        name_DATA_SIZE = ABSOLUTE(name_DATA_END - name_DATA_START);
        name_TEXT_START = .;
        build/p1-jp/name/asm/p1-jp/name/name.s.o(.text);
        name_TEXT_END = .;
        name_TEXT_SIZE = ABSOLUTE(name_TEXT_END - name_TEXT_START);
        name_RODATA_START = .;
        build/p1-jp/name/asm/p1-jp/name/name.s.o(.rodata);
        name_RODATA_END = .;
        name_RODATA_SIZE = ABSOLUTE(name_RODATA_END - name_RODATA_START);
        name_BSS_START = .;
        build/p1-jp/name/asm/p1-jp/name/name.s.o(.bss);
        name_BSS_END = .;
        name_BSS_SIZE = ABSOLUTE(name_BSS_END - name_BSS_START);
    }
    __romPos += SIZEOF(.name);
    __romPos = ALIGN(__romPos, 16);
    name_ROM_END = __romPos;
    name_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
