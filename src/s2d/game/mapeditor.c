/* S2D map editor: tile pickup/placement, preview model and debug map dump. */
#include <persona/s2d/s2d.h>
#include <libetc.h>

extern u_int   g_model_objs[];
extern int     D_800A4D1C, D_800A4D20, D_800A4D34, D_800A4D38;
extern short   D_800B8FD4, D_800B8FD8;
extern u_short D_800B91E0, D_800B91E4;
extern char    D_80064F38[], D_80064F48[], D_80064F54[];
extern int     func_8008E158(int);
extern void    S2dMapPageCheck(void);
extern int     PCinit(void), PCcreat(char*, int), PCwrite(int, void*, int), PCclose(int);

/* Views of existing fields preserve the original byte reads and store lifetimes. */
extern u_char  selected_index __asm__("D_800A4D34");
extern GsDOBJ2 preview_model __asm__("D_800B0F1C+4");
extern SVECTOR preview_rotation __asm__("D_800B0F1C+0x64");

/* The map begins with its tile-data offset; each row holds 200 halfwords. */
#define EDITOR_CELL                                                                                \
    (*(unsigned short*)(((unsigned char*)0x80122000) +                                             \
                        (D_800B8FD4 * 2 +                                                          \
                         (D_800B8FD8 * 400 + *(unsigned int*)((unsigned char*)0x80122000)))))

void func_8008C34C(void)
{
    unsigned long * attributes, saved_attribute;
    short           next_y, previous_y, next_x, previous_x;
    int             fd, tile_flags, value;
    int             remaining, chunk;
    unsigned char*  data;
    unsigned short  value16;
    unsigned short* cell;
    if (func_8008E158(1) & 0x100)
    {
        saved_attribute = g_party_obj.obj.attribute;
        D_800A4D1C = 0;
        D_800A4D20 = 0;
        g_party_obj.obj.attribute = saved_attribute & 0x7FFFFFFF;
    }
    /* The original host/debug export writes the loaded map in 32-KiB chunks. */
    if (func_8008E158(1) & 0x800)
    {
        remaining = 0x1DE90;
        data = (unsigned char*)0x80122000;
        PCinit();
        fd = PCcreat(D_80064F38, 0);
        chunk = 0x8000;
        do
        {
            PCwrite(fd, data, 0x8000);
            remaining -= chunk;
            data += chunk;
        } while ((remaining - chunk) > 0);
        PCwrite(fd, data, remaining);
        PCclose(fd);
    }
    if (func_8008E158(2) & 4)
    {
        value = (D_800A4D34 & 0xFF);
        value--;
        if (value < 0)
            value = 0;
        D_800A4D34 = value | (D_800A4D34 & 0xFF00);
    }
    if (func_8008E158(2) & 1)
    {
        value = (D_800A4D34 & 0xFF);
        value++;
        if (value > 256)
            value = 256;
        D_800A4D34 = value | (D_800A4D34 & 0xFF00);
    }
    FntPrint(D_80064F48, g_map_side);
    FntPrint(D_80064F54, g_model_objs[selected_index]);
    value = selected_index;
    if (value != 0xFF)
    {
        D_800B0F1C.kind = 1;
        g_party_obj.obj.attribute |= 0x80000000;
        {
            unsigned int model = g_model_objs[value];
            if (model != ~0U)
            {
                if (!(model & 0xC0000000))
                {
                    preview_model.tmd = (u_long*)((model & 0x3FFFFFFF) | 0x80000000);
                }
                else if (g_map_side == (value = model >> 30) - 1)
                {
                    preview_model.tmd = (u_long*)((model & 0x3FFFFFFF) | 0x80000000);
                }
                else
                {
                    preview_model.tmd = (u_long*)((g_model_objs[0] & 0x3FFFFFFF) | 0x80000000);
                }
            }
            else
            {
                preview_model.tmd = (u_long*)((g_model_objs[0] & 0x3FFFFFFF) | 0x80000000);
            }
        }
        /* The embedded coordinate begins 12 bytes after the coordinate link. */
        value = (int)&preview_model.coord2;
        tile_flags = D_800A4D34;
        preview_model.coord2 = (GsCOORDINATE2*)(value + 12);
        value16 = (u16)g_map_xform.rot.vx;
        preview_rotation.vx = 0U;
        D_800B0F1C.rot.vz = 0;
        tile_flags <<= 2;
        tile_flags &= 0xC00;
        D_800B0F1C.rot.vy = (s16)tile_flags;
        D_800B0F1C.rot.vx = value16;
        D_800B0F1C.trans = g_map_xform.trans;
    }
    else
    {
        D_800B0F1C.kind = 0;
        attributes = &g_party_obj.obj.attribute;
        *attributes = *attributes & 0x7FFFFFFF;
    }
    if (func_8008E158(1) & 0x10)
    {
        if (D_800A4D38 == 0)
        {
            cell = &EDITOR_CELL;
            value16 = *cell;
            tile_flags = value16 & 255;
            D_800A4D34 = value16;
            if (tile_flags == 255)
                D_800A4D34 = tile_flags;
            else
            {
                *cell = 255;
                D_800A4D38 = 1;
            }
        }
        else
        {
            /* Save the held tile before clearing the editor selection. */
            unsigned short saved_tile = D_800A4D34;
            D_800A4D34 = 255;
            D_800A4D38 = 0;
            EDITOR_CELL = saved_tile;
        }
    }
    if (func_8008E158(1) & 0x80)
        EDITOR_CELL ^= 0x400;
    if (func_8008E158(1) & 0x20)
        EDITOR_CELL = D_800A4D34;
    if (func_8008E158(1) & 0x40)
        EDITOR_CELL = 255;
    if (func_8008E158(1) & 8)
    {
        {
            unsigned int changed, preserved;
            changed = ((s32)D_800A4D34 >> 8) & 3;
            preserved = D_800A4D34 & ~0x300;
            changed = changed + 1;
            D_800A4D34 = ((changed & 3) << 8) | preserved;
        }
    }
    if (func_8008E158(1) & 2)
    {
        {
            unsigned int tile = D_800A4D34, changed, preserved;
            changed = ((s32)tile >> 15) & 1;
            changed = changed ^ 1;
            preserved = tile & 0xFFFF7FFF;
            D_800A4D34 = ((changed & 1) << 15) | preserved;
        }
    }
    D_800B91E0 = (u16)D_800B8FD4;
    D_800B91E4 = (u16)D_800B8FD8;
    if (func_8008E158(2) & 0x1000)
    {
        next_y = (u16)D_800B8FD8 + 1;
        D_800B8FD8 = next_y;
        if (next_y >= 0xC8)
        {
            D_800B8FD8 = 0;
        }
    }
    if (func_8008E158(2) & 0x4000)
    {
        previous_y = D_800B8FD8 - 1;
        D_800B8FD8 = previous_y;
        if (previous_y < 0)
        {
            D_800B8FD8 = 0xC7;
        }
    }
    if (func_8008E158(2) & 0x2000)
    {
        next_x = (u16)D_800B8FD4 + 1;
        D_800B8FD4 = next_x;
        if (next_x >= 0xC8)
        {
            D_800B8FD4 = 0;
        }
    }
    if (func_8008E158(2) & 0x8000)
    {
        previous_x = D_800B8FD4 - 1;
        D_800B8FD4 = previous_x;
        if (previous_x < 0)
        {
            D_800B8FD4 = 0xC7;
        }
    }
    S2dMapPageCheck();
}
