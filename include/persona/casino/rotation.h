#ifndef PERSONA_CASINO_ROTATION_H
#define PERSONA_CASINO_ROTATION_H

#include <decomp/types.h>

typedef struct CasinoRotationResponse
{
    u_char pad00[0x50];
    double angle;
    u_char pad58[0x40];
    double step;
    u_char padA0[0x18];
    double velocity;
    u_char padC0[0x20];
    s8 mode;
    u_char padE1;
    u_char snap;
    s8 band;
    s8 previous_band;
} CasinoRotationResponse;

short func_80091B78(CasinoRotationResponse *value);
void func_80091D0C(CasinoRotationResponse *value);

#endif
