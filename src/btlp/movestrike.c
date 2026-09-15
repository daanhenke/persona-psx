/* Persona 1 (JP) - the objects a move's strike is played out with.  BTLP only.
 *   0x800B66FC BtlSpawnMoveStrike
 *
 * Stages the effect's artwork the way BtlStartMoveFx does - the message comes
 * down, the loader's first run is copied to the stage and bound, and its tim
 * is uploaded - then builds the strike itself at `pos` out of the staged
 * artwork's scripts. Most moves are a single object on the set's scripts. Five
 * throw four objects out along g_btl_strike_spread first and put the last on
 * the first script table; one does the same on the set's own; and one puts
 * five out at even steps round a circle and nothing after them.
 *
 * Answers the last object made.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/fieldmarks.h>
#include <persona/btlp/gfx.h>
#include <persona/btlp/load.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/text.h>

/* Where the effect's artwork is staged, and how much of it there is. */
#define FX_GFX_STAGE 0x801D9400
#define FX_GFX_BYTES 0x1000

/* The page and slot the effect's tim goes to. */
#define FX_TIM_PAGE 0x1D
#define FX_TIM_SLOT 0xE

#define FX_KIND_GFX 3
#define FX_SE_BANK  4

/* What every strike object is made with. */
#define STRIKE_GROUP 2
#define STRIKE_DRAW  5
#define STRIKE_ATTR  0x2001

/* The spread: four objects, and the frame count each is put on. */
#define STRIKE_SPREAD      4
#define STRIKE_SPREAD_ATTR 0x600C2000

/* The circle: five objects a fifth of the way round from each other, starting
   at 0x180 and lifted 0x18 units. */
#define STRIKE_RING       5
#define STRIKE_RING_START 0x180
#define STRIKE_RING_STEP  0x40
#define STRIKE_RING_TIMER 8
#define STRIKE_RING_ATTR  0xC2000
#define STRIKE_RING_LIFT  0x180000

BtlObj *BtlSpawnMoveStrike(int move, int set, const long *pos)
{
    BtlObj *o;
    u_char *stage;
    int     i;
    int     n;

    BtlCloseMessage(0);
    g_btl_fx_gfx = (u_char *)FX_GFX_STAGE;
    stage = g_load_stage_1;
    memcpy((u_char *)FX_GFX_STAGE, stage, FX_GFX_BYTES);
    BtlBindGfx(FX_KIND_GFX, move, &g_btl_fx_gfx);
    stage = g_load_stage;
    BtlUploadTim((u_long *)stage, FX_TIM_PAGE, FX_TIM_SLOT, 1, 0, 1);
    g_btl_fx_move = g_btl_actors[g_btl_actor_turn].move;
    g_btl_fx_target = g_btl_actors[g_btl_actor_turn].order;

    switch (move) {
    case 0xAF:
    case 0xBD:
    case 0xCA:
    case 0xD4:
    case 0xDE:
        g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[1];
        for (i = 0, n = 2; i < STRIKE_SPREAD; i++, n += 2) {
            o = BtlObjAlloc(&g_btl_fx_def, STRIKE_GROUP, 0, STRIKE_DRAW, 0, pos,
                            FX_TIM_PAGE, FX_TIM_SLOT);
            o->attr |= STRIKE_SPREAD_ATTR | STRIKE_ATTR;
            o->shift_x = g_btl_strike_spread[n - 2] << 16;
            o->shift = g_btl_strike_spread[n - 1] << 16;
            o->timer = n;
            o->kind = move;
        }
        g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
        o = BtlObjAlloc(&g_btl_fx_def, STRIKE_GROUP, 0, STRIKE_DRAW, 0, pos,
                        FX_TIM_PAGE, FX_TIM_SLOT);
        o->attr |= STRIKE_ATTR;
        o->kind = move;
        break;
    case 0xD3:
        g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[set];
        for (i = 0, n = 2; i < STRIKE_SPREAD; i++, n += 2) {
            o = BtlObjAlloc(&g_btl_fx_def, STRIKE_GROUP, 0, STRIKE_DRAW, 0, pos,
                            FX_TIM_PAGE, FX_TIM_SLOT);
            o->attr |= STRIKE_SPREAD_ATTR | STRIKE_ATTR;
            o->shift_x = g_btl_strike_spread[n - 2] << 16;
            o->shift = g_btl_strike_spread[n - 1] << 16;
            o->timer = n;
            o->kind = move;
        }
        g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[set];
        o = BtlObjAlloc(&g_btl_fx_def, STRIKE_GROUP, 0, STRIKE_DRAW, 0, pos,
                        FX_TIM_PAGE, FX_TIM_SLOT);
        o->attr |= STRIKE_ATTR;
        o->kind = move;
        break;
    case 0xBB:
        g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
        for (i = 0, n = STRIKE_RING_START; i < STRIKE_RING; i++) {
            o = BtlObjAlloc(&g_btl_fx_def, STRIKE_GROUP, 0, STRIKE_DRAW, 0, pos,
                            FX_TIM_PAGE, FX_TIM_SLOT);
            o->angle = n & 0x1FF;
            o->timer = STRIKE_RING_TIMER;
            o->kind = move;
            o->z -= STRIKE_RING_LIFT;
            o->attr |= STRIKE_RING_ATTR | STRIKE_ATTR;
            n += STRIKE_RING_STEP;
        }
        break;
    default:
        g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[set];
        o = BtlObjAlloc(&g_btl_fx_def, STRIKE_GROUP, 0, STRIKE_DRAW, 0, pos,
                        FX_TIM_PAGE, FX_TIM_SLOT);
        o->attr |= STRIKE_ATTR;
        o->kind = move;
        break;
    }
    BtlSePlay(FX_SE_BANK, 0);
    return o;
}
