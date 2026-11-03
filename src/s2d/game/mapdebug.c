/* Persona 1 (JP): map debug menu input and field selectors. */
extern int fade_enabled __asm__("g_fade+8");
typedef int s32;
typedef unsigned int u32;
typedef short s16;
typedef unsigned short u16;
typedef unsigned char u8;
extern void SoundCheckMarkMap(int), func_8008BBD8(void);
extern int func_8008BCC8(int, int, int);
extern unsigned int func_8008E158(int);
extern unsigned int D_800A4D1C;
extern int D_800A4D20, D_800B5F3C;
extern short g_map_id, g_s2d_exit;
extern struct
{
    char pad[8];
    int field8;
} g_fade;
void func_8008BD78(void)
{
    int var_a2;
    s16 var_v0;
    u32 temp_v0;
    u32 temp_v0_2;
    u8 *var_s0;
    u8 var_a0;

    switch (D_800A4D1C)
    {
    case 0:
        if (func_8008E158(1) & 32)
        {
            g_s2d_exit = 0;
            D_800B5F3C = 0;
        }
        break;
    case 1:
        if (func_8008E158(1) & 32)
        {
            g_s2d_exit = 1;
            D_800B5F3C = 0;
        }
        break;
    case 2:
        if (func_8008E158(1) & 32)
        {
            SoundCheckMarkMap(g_map_id);
            g_s2d_exit = 3;
            D_800B5F3C = 0;
        }
        break;
    case 3:
        if (func_8008E158(1) & 32)
        {
            g_s2d_exit = 2;
            D_800B5F3C = 0;
        }
        break;
    case 4:
        *(u16 *)0x801F5350 = func_8008BCC8(*(u16 *)0x801F5350, 0, 0xFFFF);
        break;
    case 5:
        *(u8 *)0x801f5354 = func_8008BCC8(*(u8 *)0x801f5354, 0, 255);
        break;
    case 6:
        *(u8 *)0x801f5355 = func_8008BCC8(*(u8 *)0x801f5355, 0, 255);
        break;
    case 7:
        *(u8 *)0x801f5352 = func_8008BCC8(*(u8 *)0x801f5352, 0, 199);
        break;
    case 8:
        *(u8 *)0x801f5353 = func_8008BCC8(*(u8 *)0x801f5353, 0, 199);
        break;
    case 9:
        func_8008BBD8();
        break;
    case 10:
        if (func_8008E158(1) & 0x20)
        {
            fade_enabled = (s32)(fade_enabled != 1);
        }
        break;
    case 11:
        if (func_8008E158(1) & 0x20)
        {
            D_800A4D1C = 0;
            D_800A4D20 += 1;
        }
        break;
    }
    if (func_8008E158(1) & 0x100)
    {
        D_800A4D1C = 0;
        D_800A4D20 += 1;
    }
    if (func_8008E158(1) & 0x40)
    {
        D_800A4D1C = 0;
        D_800A4D20 += 1;
    }
    if (func_8008E158(2) & 0x1000)
    {
        temp_v0 = D_800A4D1C - 1;
        D_800A4D1C = temp_v0;
        if ((s32)temp_v0 < 0)
        {
            D_800A4D1C = 0xB;
        }
    }
    if (func_8008E158(2) & 0x4000)
    {
        temp_v0_2 = D_800A4D1C + 1;
        D_800A4D1C = temp_v0_2;
        if ((s32)temp_v0_2 >= 0xC)
        {
            D_800A4D1C = 0;
        }
    }
}
