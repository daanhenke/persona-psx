/* Persona 1 (JP) - initialize four double-precision parameters. */
#include <decomp/types.h>
extern int rand(void);
typedef struct CasinoRandomParams {
    u8 pad00[0x98];
    double value98;
    u8 padA0[0x10];
    double valueB0;
    double valueB8;
    double valueC0;
} CasinoRandomParams;
void func_80092430(CasinoRandomParams *values)
{
    values->valueC0 = -120.49;
    values->valueB0 = (rand() % 1200 - 500) * 0.01;
    values->valueB8 = (rand() % 100 - 50) * 0.1 + 40.0;
    values->value98 = (rand() % 1750 + 50) * 0.1;
}
