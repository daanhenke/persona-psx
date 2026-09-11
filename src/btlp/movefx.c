/* Persona 1 (JP) - starting the effect a move is played out with.  BTLP only.
 *   0x800B6528 BtlStartMoveFx
 *
 * Takes the message down, stages the effect's artwork, and hands the move to
 * whichever of g_btl_spell_fx's handlers it belongs to. The record's first
 * handler is the one that builds the effect; it answers the head of a chain of
 * objects, and every one of them is put on the same index, the same motion and
 * the same graphics before the swing is sounded.
 *
 * Which move is being played out and what it is aimed at are left in two bytes
 * of their own rather than passed along, because the handlers are reached
 * through a table and all take no arguments.
 *
 * A move whose record carries no handler leaves the answer untouched, which is
 * not the same as answering nothing - the caller is handed whatever the last
 * chain was. Nothing appears to depend on it.

 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/common/spell.h>

/* What the loader left at 0x80140000: one address per run it read. The effect
   takes the second of them and the tim from the first. */
extern u_char *g_load_stage;
extern u_char *D_80140004;

/* Where the effect's artwork is staged, and how much of it there is. */
#define FX_GFX_STAGE 0x801D9400
#define FX_GFX_BYTES 0x1000

/* The page and slot the effect's tim goes to. */
#define FX_TIM_PAGE 0x1D
#define FX_TIM_SLOT 0xE

/* What every object of the chain is put on, and the swing it is sounded with. */
#define FX_KIND_GFX 3
#define FX_MOTION   2
#define FX_SE_BANK  4

extern void    BtlCloseMessage(int slot);
extern int     BtlBindGfx(u_int kind, int index, u_char **image);
extern u_long *BtlUploadTim(u_long *tim, int page, int slot, int abr, int y,
                            int upload);

BtlObj *BtlStartMoveFx(int index)
{
    BtlObj *o;
    BtlObj *p;
    u_char *gfx;
    u_char *stage;
    BtlObj *(*start)();

    BtlCloseMessage(0);
    g_btl_fx_gfx = (u_char *)FX_GFX_STAGE;
    /* One local carries both of the loader's addresses in turn, which is what
       gives the block move a copy of the first rather than the load itself. */
    stage = D_80140004;
    memcpy((u_char *)FX_GFX_STAGE, stage, FX_GFX_BYTES);
    BtlBindGfx(FX_KIND_GFX, index, &g_btl_fx_gfx);
    stage = g_load_stage;
    BtlUploadTim((u_long *)stage, FX_TIM_PAGE, FX_TIM_SLOT, 1, 0, 1);

    g_btl_fx_move = g_btl_actors[g_btl_actor_turn].move;
    g_btl_fx_target = g_btl_actors[g_btl_actor_turn].order;
    start = g_btl_spell_fx[g_btl_fx_move].start;
    if (start != 0) {
        o = start();
        if (o != 0) {
            p = o;
            do {
                /* Taken again at the top of every turn; hoisted above the
                   loop the stores come out in the wrong order. */
                gfx = g_btl_unused_gfx;
                p->kind = index;
                p->motion = FX_MOTION;
                p->scripts = (const u_long **)gfx;
                p = p->attached;
            } while (p != 0);
        }
    }
    BtlSePlay(FX_SE_BANK, 0);
    return o;
}
