#include <persona/s2d/s2d.h>

extern VECTOR D_800B0D1C;
extern u8 transition_step __asm__("D_800A4CFC+0x2");
extern VECTOR map_translation __asm__("g_map_xform+0x8");
extern SVECTOR map_rotation __asm__("g_map_xform");
extern VECTOR map_scale __asm__("g_map_xform+0x18");
extern s16 D_800B0D2C;
extern u16 D_800B0D30;
extern s32 g_key_lock;
extern void func_80096224(void);
extern void func_80096514(void);
extern void func_800967DC(void);
extern void func_8008D988(VECTOR*, SVECTOR*, VECTOR*, GsCOORDINATE2*);

void func_8008AEA8(void)
{
    VECTOR offset;
    s16 phase = D_800B0D2C;
    if (phase > 0)
    {
        offset.vx = (phase * D_800B0D1C.vx) >> 12;
        offset.vy = (phase * D_800B0D1C.vy) >> 12;
        offset.vz = (phase * D_800B0D1C.vz) >> 12;
        g_key_lock = 0;
        D_800B0D2C = phase - D_800B0D30;
        if (D_800B0D2C <= 0)
        {
            g_key_lock = 1;
            transition_step++;
            if (transition_step == 16)
            {
                u8* save_cycle = (u8*)0x801F2B30;
                transition_step = 0;
                *save_cycle = (*save_cycle + 1) & 15;
            }
            func_80096224();
            func_80096514();
        }
    }
    else
    {
        offset.vx = 0;
        offset.vy = 0;
        offset.vz = 0;
    }
    offset.vx += map_translation.vx;
    offset.vy += map_translation.vy;
    offset.vz += map_translation.vz;
    func_8008D988(&offset, &map_rotation, &map_scale, &g_s2d_cam_coord);
    func_8008D988((VECTOR*)&g_s2d_cam_x, (SVECTOR*)&g_s2d_cam_rx, &g_s2d_cam_scale, &g_root_coord);
    func_800967DC();
}
