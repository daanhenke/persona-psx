/* Persona 1 (JP) - integrate a double accumulator and return its short value. */
#include <decomp/types.h>
typedef struct {
    u8 pad00[0x68];
    double value68;
    u8 pad70[0x40];
    double valueB0;
} CasinoDoubleStep;
short func_80091B00(CasinoDoubleStep *values)
{
    values->value68 = 0.1 * values->valueB0 + values->value68;
    return values->value68;
}
