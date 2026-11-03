#ifndef PERSONA_CASINO_BOUNCE_H
#define PERSONA_CASINO_BOUNCE_H

#include <decomp/types.h>

typedef struct CasinoBounce
{
    u8 pad00[0x78];
    double value78;
    u8 pad80[0x18];
    double value98;
    u8 padA0[0x20];
    double valueC0;
    u8 padC8[0x18];
    s8 mode;
    u8 padE1;
    u8 inhibit;
} CasinoBounce;

void func_8009190C(CasinoBounce* value);

#endif
