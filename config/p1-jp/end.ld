SECTIONS
{
    HIDDEN(__romPos = 0);
    header_ROM_START = __romPos;
    header_VRAM = ADDR(.header);
    .header 0x80080000 : AT(header_ROM_START) SUBALIGN(16)
    {
        FILL(0x00000000);
        header_DATA_START = .;
        build/p1-jp/end/asm/p1-jp/end/header.s.o(.data);
        header_DATA_END = .;
        header_DATA_SIZE = ABSOLUTE(header_DATA_END - header_DATA_START);
    }
    __romPos += SIZEOF(.header);
    header_ROM_END = __romPos;
    header_VRAM_END = .;

    end_ROM_START = __romPos;
    end_VRAM = ADDR(.end);
    .end 0x80080000 : AT(end_ROM_START) SUBALIGN(16)
    {
        FILL(0x00000000);
        end_TEXT_START = .;
        build/p1-jp/end/asm/p1-jp/end/end.s.o(.text);
        . = ALIGN(., 16);
        end_TEXT_END = .;
        end_TEXT_SIZE = ABSOLUTE(end_TEXT_END - end_TEXT_START);
        end_DATA_START = .;
        build/p1-jp/end/asm/p1-jp/end/end.s.o(.data);
        . = ALIGN(., 16);
        end_DATA_END = .;
        end_DATA_SIZE = ABSOLUTE(end_DATA_END - end_DATA_START);
        end_RODATA_START = .;
        build/p1-jp/end/asm/p1-jp/end/end.s.o(.rodata);
        . = ALIGN(., 16);
        end_RODATA_END = .;
        end_RODATA_SIZE = ABSOLUTE(end_RODATA_END - end_RODATA_START);
        end_BSS_START = .;
        build/p1-jp/end/asm/p1-jp/end/end.s.o(.bss);
        . = ALIGN(., 16);
        end_BSS_END = .;
        end_BSS_SIZE = ABSOLUTE(end_BSS_END - end_BSS_START);
    }
    __romPos += SIZEOF(.end);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    end_ROM_END = __romPos;
    end_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
