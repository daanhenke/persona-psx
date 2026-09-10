/* Persona 1 (JP) - reading a party member's model off the disc.  BTLP only.
 *   0x80083D58 BtlLoadMemberGfx
 *
 * Answers with the VRAM slot the model went into. Slots 5 to 9 belong to the
 * members: one already owned by this member is reused, otherwise the last free
 * one is taken, and with none free the answer keeps the 0x8000 it started with.
 *
 * The file's extent is the pair of offsets either side of the member's entry in
 * g_btl_member_file. It is read to the shared staging buffer, copied from there
 * to wherever g_btl_gfx_next points, and bound in place. The halfword at +2 of
 * the file is the palette's length - it is kept on the actor for BtlStepCluts
 * and then zeroed, because the TIM upload that follows reads the same halfword
 * as part of the header.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <libcd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/sound.h>

/* Slots 5 to 9 are the party's; -1 is a free one and 0x8000 the answer when
   none of them can be had. */
#define BTL_MEMBER_SLOT0 5
#define BTL_MEMBER_SLOT1 10
#define BTL_SLOT_FREE    (-1)
#define BTL_SLOT_NONE    0x8000
#define BTL_SLOT_MASK    0x7FFF

/* Members past the ninth are the same models over again. */
#define BTL_MEMBER_WRAP 10

/* A palette is 0x200 bytes, one page per actor. */
#define BTL_CLUT_BYTES 0x200

/* The staging buffer every loader reads into, by address. */
#define BTL_STAGE ((u_long **)0x80140000)

/* g_cd_busy once the read is over. */
#define CD_IDLE (-1)

extern u_short   g_btl_member_file[];
extern int       g_btl_gfx_sector;
extern u_char   *g_btl_gfx_next;
extern u_long   *g_btl_slot_clut[];
extern u_char   *g_btl_actor_clut;
extern u_char   *g_btl_actor_clut_to;
extern u_char   *g_btl_actor_clut_base;
extern volatile int g_cd_busy;

extern void    CdReadFileToAddrAsync(CdlFILE *file, int sectors, u_long *dest);
extern int     BtlBindGfx(u_int kind, int index, u_char **image);
extern u_long *BtlUploadTim(u_long *tim, int page, int slot, int abr, int y,
                            int put);

u_short BtlLoadMemberGfx(int member, int actor)
{
    CdlLOC   loc;
    u_long  *tim;
    u_long  *clut;
    u_short  slot;
    u_short  len;
    u_short  i;

    slot = BTL_SLOT_NONE;
    if (member >= BTL_MEMBER_WRAP) {
        member -= BTL_MEMBER_WRAP;
    }
    i = BTL_MEMBER_SLOT0;
    while (i < BTL_MEMBER_SLOT1) {
        if (g_btl_slot_owner[i] == member) {
            slot = i;
            break;
        }
        if (g_btl_slot_owner[i] == BTL_SLOT_FREE) {
            slot = i;
        }
        i++;
    }
    g_btl_slot_owner[slot] = member;

    CdIntToPos(g_btl_member_file[member] + g_btl_gfx_sector, &loc);
    CdReadFileToAddrAsync((CdlFILE *)&loc,
                          g_btl_member_file[member + 1]
                              - g_btl_member_file[member],
                          (u_long *)BTL_STAGE);
    while (g_cd_busy != CD_IDLE) {
        BtlDrawFrame();
    }
    memcpy(g_btl_gfx_next, (u_char *)BTL_STAGE[1],
           (int)BTL_STAGE[2] - (int)BTL_STAGE[1]);
    BtlBindGfx(0, member, &g_btl_gfx_next);

    tim = BTL_STAGE[0];
    len = ((u_short *)tim)[1];
    g_btl_actors[actor].clut_len = len;
    ((u_short *)tim)[1] = 0;
    clut = BtlUploadTim(tim, slot, actor, 0, 0, 1);
    g_btl_slot_clut[slot] = clut;
    memcpy(g_btl_actor_clut + actor * BTL_CLUT_BYTES, (u_char *)clut,
           BTL_CLUT_BYTES);
    /* And again into the two the fades work between, so the actor starts out
       at rest with nothing to walk toward. */
    memcpy(g_btl_actor_clut_to + actor * BTL_CLUT_BYTES,
           (u_char *)g_btl_slot_clut[slot], BTL_CLUT_BYTES);
    memcpy(g_btl_actor_clut_base + actor * BTL_CLUT_BYTES,
           (u_char *)g_btl_slot_clut[slot], BTL_CLUT_BYTES);
    return slot & BTL_SLOT_MASK;
}
