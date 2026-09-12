/* Persona 1 (JP) - the move drawn as three records spread across the acting
 * side, with the arena thrown white behind them.  BTLP only.
 *   0x800B9638 BtlFxStart2F
 *
 * A start handler out of g_btl_spell_fx. The arena is put on full white and
 * told to walk there quickly, and then three records are opened in a row
 * across the side that is acting - sixty units apart, a hundred and sixty out
 * from the middle of the field - each threaded onto the one before it. The
 * last of the three is the one the table is answered with, which is the near
 * end of the row rather than the far one.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* What the arena is put on, and how much of the gap it closes each frame. */
#define FX_2F_FULL 0xFF
#define FX_2F_FADE 0x60

/* How far out from the middle of the field the row stands and how far apart
   its records are - 16.16, so forty units out and sixty between. The party's
   side is the negative one. */
#define FX_2F_OFF    0x280000
#define FX_2F_SPREAD 0x3C0000

/* What each record starts as: the plain effect attribute and one bit more. */
#define FX_2F_ATTR (FX_OBJ_ATTR | 0x40000)

/* How long each record waits before it arrives, in frames, counted down with
   the record's own number. */
#define FX_2F_WAIT 4

/* How many records the row is made of, counted down. */
#define FX_2F_LAST 2

BtlObj *BtlFxStart2F(void)
{
    BtlObj *o;
    BtlObj *prev;
    BtlObj *head;
    long    pos[3];
    int     i;

    g_btl_arena_rgb[0] = FX_2F_FULL;
    g_btl_arena_rgb[1] = FX_2F_FULL;
    g_btl_arena_rgb[2] = FX_2F_FULL;
    g_btl_arena_fade = FX_2F_FADE;
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    i = FX_2F_LAST;
    prev = NULL;
    do {
        pos[1] = (g_btl_actor_turn < BTL_PARTY) ? -FX_2F_OFF : FX_2F_OFF;
        pos[0] = (i - 1) * FX_2F_SPREAD;
        pos[2] = 0;
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, prev, FX_OBJ_DRAW, 0, pos,
                        FX_OBJ_CD, FX_OBJ_CE);
        o->attr = FX_2F_ATTR;
        o->attached = prev;
        prev = o;
        o->timer = i + FX_2F_WAIT;
        o->mark_num = i;
        if (i == 0) {
            head = o;
        }
    } while (--i >= 0);
    return head;
}
