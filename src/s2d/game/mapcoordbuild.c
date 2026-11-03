#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern MATRIX D_8005E6B0;

void func_8008D988(VECTOR* position, SVECTOR* rotation, VECTOR* scale, GsCOORDINATE2* out)
{
    MATRIX matrix = D_8005E6B0;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, position);
    ScaleMatrix(&matrix, scale);
    out->coord = matrix;
    out->flg = 0;
}
