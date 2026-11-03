#ifndef PERSONA_CASINO_GRID_H
#define PERSONA_CASINO_GRID_H
#include <decomp/types.h>

typedef struct { u_char a, b, c; } CasinoSymbolRow;
typedef struct { CasinoSymbolRow rows[3]; } CasinoSymbolBlock;
typedef struct { u8 digit[4][3]; } CasinoDigitRows;
typedef struct { s8 cells[3][3]; } TargetGrid;

short func_8008CBF8(s8 cells[3][3], TargetGrid goal);
void func_8008AAF8(int index, CasinoSymbolBlock symbols);
void func_8008ABD8(u8 index, CasinoDigitRows rows);
#endif
