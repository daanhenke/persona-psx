#include <decomp/types.h>

typedef struct
{
    double x, y, z;
} CasinoDoubleVector;
void func_80092C44(CasinoDoubleVector value, s16 *out)
{
    out[0] = (int)value.x;
    out[1] = (int)value.y;
    out[2] = (int)value.z;
}
