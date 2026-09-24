/* Persona 1 (JP) - writing the battle back into the save block.  BTLP only.
 *   0x80086884 BtlStoreParty
 *
 * The other end of BtlTakeParty: whatever the fight did to the five party
 * records is put back where the field will find it, and the settings the
 * battle kept its own copies of go back with them.
 *
 * Each actor's whole Char goes to the record g_party names for that slot, so a
 * member who was healed, hurt or given an ailment carries it out of the fight.
 * The formation follows - all eight saved layouts and then the one the party
 * actually stood in, which the battle keeps separately and which lands in the
 * ninth row the field reads as the live one.
 *
 * Nothing here allocates a frame: it is all copies, and the compiler has the
 * temporaries it needs in the caller-saved registers.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/common/char.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/menu.h>
#include <persona/common/formation.h>
#include <persona/common/status.h>

/* The eight layouts the player can store, and the ninth row the field treats
   as the one they are standing in. */
#define FORM_PRESETS 8
#define FORM_LIVE    8

/* One byte per party slot, taken off the actor: g_options + 0x1E, the slot
   BtlTakeParty reads back. A number rather than a name - the image adds the
   index first, as maspsx does for a literal base (takeparty's load does the
   same). sym.btlp.txt marks this and the three settings below ignore:True,
   so splat leaves them as numbers in the image's asm as well. */
#define g_save_actor_flag ((u_char *)0x801F2AE6)

/* Where they live in the save block, beside the HUD style hudload.c reaches
   the same way. The five bytes before the animation setting take one byte per
   party slot off the actor; what BtlActor +0xC8 means is not settled, only
   that the battle takes it from here and hands it back. */
#define g_save_fast_anim  (*(u_char *)0x801F2AEB)
#define g_save_confirm    (*(u_char *)0x801F2AC9)
#define g_save_msg_speed  (*(u_char *)0x801F2ACA)

/* 99.76%. The party is walked by a pointer of its own. Indexed by i, loop.c
   makes the flag's address a constant offset from the party's, and the flag
   stops being indexed by i itself. One pair is left: the image loads g_chars
   before the party pointer, and here the pointer's init comes first. A
   source init sits ahead of anything loop.c hoists, so the image's pointer
   looks like a reduced giv. */
#ifdef NON_MATCHING
void BtlStoreParty(void)
{
    Char *c;
    u_char *party;
    int i;

    i = 0;
    party = g_party;
    for (; i < BTL_PARTY; party++, i++) {
        /* The destination is worked out before the flag is stored; the
           other way round gcc schedules the store first. */
        c = &g_chars[*party];
        g_save_actor_flag[i] = g_btl_actors[i].tactic;
        memcpy(c, &g_btl_actors[i].c, sizeof(Char));
    }
    memcpy(g_formation_preset, g_btl_formation_preset,
           GRID_CELLS * FORM_PRESETS);
    memcpy(&g_formation_preset[GRID_CELLS * FORM_LIVE], g_btl_formation,
           GRID_CELLS);
    g_save_fast_anim = g_btl_fast_anim;
    g_save_confirm = g_btl_confirm;
    g_save_msg_speed = g_btl_msg_speed;
}
#else
INCLUDE_ASM("btlp/nonmatchings/storeparty", BtlStoreParty);
#endif
