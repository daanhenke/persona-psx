SECTIONS
{
    HIDDEN(__romPos = 0);
    header_ROM_START = __romPos;
    header_VRAM = ADDR(.header);
    .header 0x80080000 : AT(header_ROM_START) SUBALIGN(16)
    {
        FILL(0x00000000);
        header_DATA_START = .;
        build/p1-jp/movie/asm/p1-jp/movie/header.s.o(.data);
        header_DATA_END = .;
        header_DATA_SIZE = ABSOLUTE(header_DATA_END - header_DATA_START);
    }
    __romPos += SIZEOF(.header);
    header_ROM_END = __romPos;
    header_VRAM_END = .;

    movie_ROM_START = __romPos;
    movie_VRAM = ADDR(.movie);
    .movie 0x80080000 : AT(movie_ROM_START) SUBALIGN(16)
    {
        FILL(0x00000000);
        movie_TEXT_START = .;
        build/p1-jp/movie/asm/p1-jp/movie/movie.s.o(.text);
        . = ALIGN(., 16);
        movie_TEXT_END = .;
        movie_TEXT_SIZE = ABSOLUTE(movie_TEXT_END - movie_TEXT_START);
        movie_DATA_START = .;
        build/p1-jp/movie/asm/p1-jp/movie/movie.s.o(.data);
        . = ALIGN(., 16);
        movie_DATA_END = .;
        movie_DATA_SIZE = ABSOLUTE(movie_DATA_END - movie_DATA_START);
        movie_RODATA_START = .;
        build/p1-jp/movie/asm/p1-jp/movie/movie.s.o(.rodata);
        . = ALIGN(., 16);
        movie_RODATA_END = .;
        movie_RODATA_SIZE = ABSOLUTE(movie_RODATA_END - movie_RODATA_START);
        movie_BSS_START = .;
        build/p1-jp/movie/asm/p1-jp/movie/movie.s.o(.bss);
        . = ALIGN(., 16);
        movie_BSS_END = .;
        movie_BSS_SIZE = ABSOLUTE(movie_BSS_END - movie_BSS_START);
    }
    __romPos += SIZEOF(.movie);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    movie_ROM_END = __romPos;
    movie_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
