/* Persona 1 (JP) - rotate the map displacement and store its scale/angle. */
#include <decomp/types.h>
#include <libgte.h>
extern SVECTOR g_map_xform;
extern VECTOR D_800B0D1C;
extern short D_800B0D2C, D_800B0D30;
extern short D_800B8FD4, D_800B8FD8, D_800B91E0, D_800B91E4;

void func_8008AE00(short angle)
{
    MATRIX matrix;
    SVECTOR displacement;
    D_800B0D2C = 0x1000;
    displacement.vy = 0;
    displacement.vx = (D_800B8FD4 - D_800B91E0) * 640;
    displacement.vz = (D_800B8FD8 - D_800B91E4) * 640;
    RotMatrix(&g_map_xform, &matrix);
    ApplyMatrix(&matrix, &displacement, &D_800B0D1C);
    D_800B0D30 = angle;
}
