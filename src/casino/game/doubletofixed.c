#include <decomp/types.h>

typedef struct
{
    double x, y, z;
} CasinoDoubleVector;
void func_80092D34(CasinoDoubleVector value, s32 *out)
{
    out[0] = (int)(4096.0 * value.x);
    out[1] = (int)(4096.0 * value.y);
    out[2] = (int)(4096.0 * value.z);
}
