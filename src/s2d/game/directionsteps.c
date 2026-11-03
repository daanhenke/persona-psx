/* Persona 1 (JP) - step map coordinates relative to the facing direction. */
#include <decomp/types.h>

extern s16 g_s2d_facing;
extern u_short D_800B8FD4, D_800B8FD8;
extern u_int D_800A4EF4;
extern int D_800A4EF8;

void func_8008CB40(void)
{
    switch (g_s2d_facing) {
    case 0:
        D_800B8FD8++;
        break;
    case 1:
        D_800B8FD4++;
        break;
    case 2:
        D_800B8FD8--;
        break;
    case 3:
        D_800B8FD4--;
        break;
    }
    D_800A4EF8 = 0;
    D_800A4EF4 &= 0xF0;
}

void func_8008CC18(void)
{
    switch (g_s2d_facing) {
    case 0:
        D_800B8FD4++;
        break;
    case 1:
        D_800B8FD8--;
        break;
    case 2:
        D_800B8FD4--;
        break;
    case 3:
        D_800B8FD8++;
        break;
    }
    D_800A4EF8 = 1;
    D_800A4EF4 = (D_800A4EF4 & 0xF0) | 1;
}

void func_8008CCF8(void)
{
    switch (g_s2d_facing) {
    case 0:
        D_800B8FD8--;
        break;
    case 1:
        D_800B8FD4--;
        break;
    case 2:
        D_800B8FD8++;
        break;
    case 3:
        D_800B8FD4++;
        break;
    }
    D_800A4EF8 = 1;
    D_800A4EF4 = (D_800A4EF4 & 0xF0) | 2;
}

void func_8008CDD8(void)
{
    switch (g_s2d_facing) {
    case 0:
        D_800B8FD4--;
        break;
    case 1:
        D_800B8FD8++;
        break;
    case 2:
        D_800B8FD4++;
        break;
    case 3:
        D_800B8FD8--;
        break;
    }
    D_800A4EF8 = 1;
    D_800A4EF4 = (D_800A4EF4 & 0xF0) | 3;
}

