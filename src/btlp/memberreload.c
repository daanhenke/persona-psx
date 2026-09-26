/* Persona 1 (JP) - reading a member's model again into the slot it owns.
 *   0x80084138 BtlReloadMemberGfx    (BTLP only)
 *
 * The same job as BtlLoadMemberGfx next door, for a member that is already on
 * the field: the slot has to exist, so this one only looks for the slot that
 * owns the member and never takes a free one. With none found there is nothing
 * sensible to do and it stops the battle where it stands, drawing frames for
 * ever - which is how the original says "this cannot happen".
 *
 * The other differences are that the file is read synchronously rather than
 * through the asynchronous path, and that nothing is bound: the model is
 * already in place, and only the palette is wanted.
 *
 * The member is wrapped past the ninth for the slot search but not for the
 * file, so a wrapped member re-reads its own entry and lands in the slot the
 * model it shares is already in.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/gfx.h>
#include <persona/btlp/clut.h>
#include <persona/btlp/load.h>

/* Slots 5 to 9 are the party's. */
#define BTL_MEMBER_SLOT0 5
#define BTL_MEMBER_SLOT1 10

/* Members past the ninth are the same models over again. */
#define BTL_MEMBER_WRAP 10

/* A palette is 0x200 bytes, one page per actor. */
#define BTL_CLUT_BYTES 0x200

/* The staging buffer every loader reads into, by address: its low half is
   zero, so one lui is the whole of it. */
#define BTL_STAGE ((u_long **)0x80140000)

extern u_short g_btl_member_file[];
extern int     g_btl_gfx_sector;

extern void    BtlReadSectors(u_long *dest, int sector, int sectors);

/* The owner's variable is used again for the file's first sector, which is
   what gives it the copy of the member the image makes, and the TIM is read
   through g_load_stage the way BtlLoadMemberGfx reads it. The wrap sits in a
   do/while (0): flow weights a register's uses by loop depth, and those three
   uses lift the owner's priority (9 uses over 17 insns, 1.59) past the slot
   walk's pointer (2), so global alloc gives the owner v1 and the pointer a0
   as the image has them. */
short BtlReloadMemberGfx(int member, int actor)
{
    u_long *tim;
    u_long *clut;
    int     owner;
    int     i;

    owner = member;
    do {
        if (owner >= BTL_MEMBER_WRAP) {
            owner -= BTL_MEMBER_WRAP;
        }
    } while (0);
    for (i = BTL_MEMBER_SLOT0; i < BTL_MEMBER_SLOT1; i++) {
        if (g_btl_slot_owner[i] == owner) {
            break;
        }
    }
    if (i >= BTL_MEMBER_SLOT1) {
        for (;;) {
            BtlDrawFrame();
        }
    }

    owner = g_btl_member_file[member];
    BtlReadSectors((u_long *)BTL_STAGE, owner + g_btl_gfx_sector,
                   g_btl_member_file[member + 1] - owner);

    /* The byte at +2 of the file is the palette's length, one short of what
       BtlStepCluts wants. Zeroing the halfword it sits in leaves the TIM
       header the upload below reads. */
    tim = (u_long *)g_load_stage;
    g_btl_actors[actor].clut_len = *((u_char *)tim + 2) + 1;
    ((u_short *)tim)[1] = 0;
    clut = BtlUploadTim(tim, i, actor, 0, 0, 1);
    g_btl_slot_clut[i] = clut;
    memcpy((u_char *)g_btl_actor_clut + actor * BTL_CLUT_BYTES, (u_char *)clut,
           BTL_CLUT_BYTES);
    /* And again into the two the fades work between, so the actor starts out
       at rest with nothing to walk toward. */
    memcpy((u_char *)g_btl_actor_clut_to + actor * BTL_CLUT_BYTES,
           (u_char *)g_btl_slot_clut[i], BTL_CLUT_BYTES);
    memcpy((u_char *)g_btl_actor_clut_base + actor * BTL_CLUT_BYTES,
           (u_char *)g_btl_slot_clut[i], BTL_CLUT_BYTES);
    return i;
}
