/* Persona 1 (JP) - the status panel's graphics.  BTLP only.
 *   0x8007DFA8 BtlHudLoad
 *
 * Three packed images expand into the staging buffers and go up as eight
 * pieces of VRAM. Two of them are the window frame, stepped 0x3A0 into their
 * bank by the frame the player chose, so the battle's panel matches the rest
 * of the game's windows.
 *
 * The transform is only put back when the panel is fully closed. Reloading the
 * graphics behind an open panel therefore leaves it where it is, which is what
 * lets the negotiation swap the artwork mid-scene; callers that want it shown
 * follow this with BtlHudShow.
 */
#include <types.h>

/* Where each image is expanded to. */
#define HUD_STAGE   ((u_char *)0x80140000)
#define FRAME_STAGE ((u_char *)0x80147000)
#define EXTRA_STAGE ((u_char *)0x8014D180)

/* Which frame the player chose, in the save-game work area. */
#define g_hud_style (*(u_char *)0x801F2AC6)

/* One frame's worth of the frame bank. */
#define FRAME_STYLE 0x3A0

/* Where the panel rests. */
#define HUD_HOME_X 0xA0
#define HUD_HOME_Y 200
#define HUD_HOME_Z 100

extern const u_char *g_btl_hud_packed;
extern const u_char *g_btl_frame_packed;
extern const u_char  g_btl_extra_packed[];
extern const u_char  g_btl_text_cluts[];

extern short  g_btl_hud_x;
extern short  g_btl_hud_y;
extern int    g_btl_hud_tx;
extern int    g_btl_hud_ty;
extern int    g_btl_hud_tz;
extern short  g_btl_hud_rx;
extern short  g_btl_hud_ry;
extern short  g_btl_hud_rz;
extern int    g_btl_hud_scale;
extern int    g_btl_hud_scale_y;
extern int    g_btl_hud_scale_z;
extern u_char g_btl_hud_state;

extern void BtlUnpack(u_char *dst, const u_char *src);
extern void BtlQueueVramLoad(const void *src, int x, int y, int w, int h);
extern void BtlIndicatorClear(void);

void BtlHudLoad(void)
{
    BtlUnpack(HUD_STAGE, g_btl_hud_packed);
    BtlUnpack(FRAME_STAGE, g_btl_frame_packed);
    BtlUnpack(EXTRA_STAGE, g_btl_extra_packed);

    BtlQueueVramLoad(g_btl_text_cluts, 0x380, 0x180, 0x10, 4);
    BtlQueueVramLoad(HUD_STAGE + 0x200, 0x300, 0x100, 0x50, 0xB0);
    BtlQueueVramLoad(HUD_STAGE, 0, 0x1FA, 0x100, 1);
    BtlQueueVramLoad(FRAME_STAGE + 0x20 + g_hud_style * FRAME_STYLE,
                     0x340, 0x1B0, 8, 0x38);
    BtlQueueVramLoad(FRAME_STAGE + g_hud_style * FRAME_STYLE, 0x300, 0x1F8, 0x10, 1);
    BtlQueueVramLoad(EXTRA_STAGE + 0x200, 0x350, 0x180, 0x20, 0x18);
    BtlQueueVramLoad(EXTRA_STAGE, 0, 499, 0x100, 1);

    g_btl_hud_x = HUD_HOME_X;
    g_btl_hud_state = 0;
    g_btl_hud_y = HUD_HOME_Y;
    if (g_btl_hud_scale == 0) {
        g_btl_hud_rx = 0;
        g_btl_hud_ry = 0;
        g_btl_hud_rz = 0;
        g_btl_hud_tx = 0;
        g_btl_hud_ty = 0;
        g_btl_hud_tz = HUD_HOME_Z;
        g_btl_hud_scale = 0;
        g_btl_hud_scale_y = 0;
        g_btl_hud_scale_z = 0;
    }
    BtlIndicatorClear();
}
