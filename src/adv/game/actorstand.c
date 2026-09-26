/* Persona 1 (JP) - the standing sprite and the one below it.  ADV only.
 *   0x80082128 AdvWaitFrame        0x80082148 AdvImageMorph
 *   0x800822CC ActorSetStandSprite 0x8008248C ActorSetShadowSprite
 *
 * An actor standing still shows one sprite out of the overlay's own set per
 * facing, and the first eight may carry a second one in the slot 24 along:
 * a shadow, a reflection or a plain copy (SHADOW_*).
 *
 * Ahead of them are two routines nothing in the overlay calls: one frame's
 * wait, and a slow morph of a picture already in VRAM into the one a loaded
 * 8-bit TIM holds.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/adv/actor.h>
#include <persona/common/slot.h>

#define g_slots      ((Slot *)0x800DC10C)
#define g_seq_handle (*(short *)0x801F5390)

/* Where the morph keeps its working copy of the picture. */
#define MORPH_BUF    ((u_char *)0x800F0000)
#define MORPH_W      0x50   /* bytes a row: 0x28 VRAM words of 8-bit pixels */
#define MORPH_H      0x5E
#define MORPH_ROWS   ((u_char (*)[MORPH_W])MORPH_BUF)

/* An 8-bit TIM's pixels start past its header, its 256-colour CLUT block and
   the pixel block's own header. */
#define TIM8_PIXELS  0x220

#define SHADOW_SLOT  24

extern Slot   *g_slot_cur;
extern void   *g_stand_defs[];   /* by actor and facing; the flat shadows
                                    follow, 32 entries on                  */
extern u_char  g_dir_flip[];
extern u_char  g_reflect_def[];
extern u_char  g_reflect_lit_def[];

extern void AdvRunFrame(void);
extern void QueueImageUpload(RECT *rect, u_long *p);
extern void SsSeqStop(short seq);

void AdvWaitFrame(void)
{
    AdvRunFrame();
}

/* Steps every byte of the picture one towards the TIM's, 256 times - enough
   to get anywhere - and sends it back to VRAM after each pass. The TIM
   pointer itself is moved on to the pixels. */
void AdvImageMorph(u_char (*tim)[MORPH_W])
{
    RECT    rect = { 0x140, 0x100, 0x28, MORPH_H };
    u_char (*from)[MORPH_W];
    short   pass;
    short   y;
    short   x;

    tim = (u_char (*)[MORPH_W])((u_char *)tim + TIM8_PIXELS);
    from = MORPH_ROWS;
    StoreImage(&rect, (u_long *)MORPH_BUF);
    AdvRunFrame();
    for (pass = 0; pass < 0x100; pass++) {
        for (y = 0; y < MORPH_H; y++) {
            if (y == 0x30) {
                AdvRunFrame();
            }
            for (x = 0; x < MORPH_W; x++) {
                if (from[y][x] < tim[y][x]) {
                    from[y][x]++;
                }
                if (from[y][x] > tim[y][x]) {
                    from[y][x] += 0xFF;
                }
            }
        }
        QueueImageUpload(&rect, (u_long *)from);
        AdvRunFrame();
    }
}

#define A g_adv_actors[a]

void ActorSetShadowSprite(short a);

void ActorSetStandSprite(u_char a)
{
    Slot *shadow = &g_slots[a + SHADOW_SLOT];
    SlotInit(g_stand_defs[a * 4 + A.dir], a, A.z, A.world_x + 4,
             (short)A.world_y - 2);
    if (a < 8) {
        ActorSetShadowSprite(a);
    }
    g_slot_cur = &g_slots[a];
    SlotSetBrightness(a, A.bright);
    g_slot_cur->clut_y = a;
    g_slot_cur->tpage_add = a;
    shadow->tpage_add = a;
    if (g_dir_flip[A.dir]) {
        g_slot_cur->attr |= SLOT_ATTR_XSCALE;
        shadow->attr |= SLOT_ATTR_XSCALE;
    }
    A.flags |= 0x80;
    if (a == 0) {
        SsSeqStop(g_seq_handle);
    }
}

void ActorSetShadowSprite(short a)
{
    Slot *shadow = &g_slots[a + SHADOW_SLOT];

    switch (A.shadow) {
    case SHADOW_FLAT:
        SlotInit(g_stand_defs[a * 4 + A.dir + 32], a + SHADOW_SLOT, A.z,
                 A.world_x, A.world_y);
        break;
    case SHADOW_FLAT_LOW:
        SlotInit(g_stand_defs[a * 4 + A.dir], a + SHADOW_SLOT, A.z,
                 A.world_x, A.world_y);
        shadow->clut_y = a;
        break;
    case SHADOW_COPY:
        SlotInitTagged(g_reflect_def, a + SHADOW_SLOT, A.z, A.world_x,
                       A.world_y);
        break;
    case SHADOW_COPY_LIT:
        SlotInitTagged(g_reflect_lit_def, a + SHADOW_SLOT, A.z, A.world_x,
                       A.world_y);
        shadow->clut_y = a;
        break;
    }
}
