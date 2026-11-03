#include <decomp/types.h>

typedef struct
{
    double x, y, z;
} CasinoDoubleVector;
void func_80092CB0(CasinoDoubleVector value, s32 *out)
{
    out[0] = (int)value.x;
    out[1] = (int)value.y;
    out[2] = (int)(762.0 - value.z);
}
