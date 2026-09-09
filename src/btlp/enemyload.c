/* Persona 1 (JP) - reading one enemy's artwork into VRAM.  BTLP only.
 *   0x80083354 BtlLoadEnemyGfx
 *
 * The enemy side of BtlLoadMemberGfx, called per enemy by BtlSpawnEnemies and
 * answering with the VRAM slot the artwork went into.
 *
 * Species 0xBB to 0xC0 are the exception and go first: their image is copied
 * and bound with no upload at all, and the answer is a fixed slot 10.
 *
 * For the rest, the species record says how much VRAM the enemy needs, and that
 * picks which run of g_btl_slot_owner is searched. A slot this species already
 * owns is reused; otherwise a free one is taken, and with none free the answer
 * keeps the 0x8000 it started with and nothing is loaded. The image is copied
 * and bound, the TIM uploaded to that slot's page, and the palette copied into
 * all three of the actor's palettes so it starts at rest.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <persona/btlp/model.h>
#include <persona/btlp/battle.h>

/* Species whose artwork is bound in place rather than uploaded. */
#define BTL_SPECIES_FLAT0 0xBB
#define BTL_SPECIES_FLATN 6
#define BTL_SLOT_FLAT     10

/* The slot runs the four size classes search, and what marks a free one. */
#define BTL_SLOT_FIRST 10
#define BTL_SLOT_LAST  0xF
#define BTL_SLOT_STEP  4
#define BTL_SLOT_FREE  (-1)
#define BTL_SLOT_NONE  0x8000

/* Which of the wide slots each size class keeps. */
#define BTL_WIDE_OWNER 10
#define BTL_WIDE0      12
#define BTL_WIDE1      13
#define BTL_WIDE2      14
#define BTL_WIDE3      15

/* What the two combined classes answer with. */
#define BTL_SLOT_PAIR 0x800C
#define BTL_SLOT_WIDE 0x800A

/* A palette is 0x200 bytes, one page per actor. */
#define BTL_CLUT_BYTES 0x200
#define BTL_SLOT_MASK  0x7FFF
#define BTL_ENEMY_SLOT0 5

/* Where the enemy CLUTs sit in VRAM, one row per actor. */
#define BTL_CLUT_Y 0x1E5

extern BtlModel  g_btl_models[];
extern short     g_btl_slot_owner[];
extern u_long   *g_btl_slot_clut[];
extern u_char   *g_btl_gfx_next;
extern u_char   *g_btl_enemy_clut;
extern u_char   *g_btl_enemy_clut_to;
extern u_char   *g_btl_enemy_clut_base;

extern u_short GetClut(int x, int y);
extern int     BtlBindGfx(u_int kind, int index, u_char **image);
extern u_long *BtlUploadTim(u_long *tim, int page, int slot, int abr, int y,
                            int put);

#ifdef NON_MATCHING
int BtlLoadEnemyGfx(int species, int actor, u_long *tim, u_char *image,
                    int bytes)
{
    u_long  *clut;
    u_long **held;
    int      size;
    u_short  found;
    short   *wide;
    short   *owner;

    found = BTL_SLOT_NONE;
    if ((u_int)(species - BTL_SPECIES_FLAT0) < BTL_SPECIES_FLATN) {
        memcpy(g_btl_gfx_next, image, bytes);
        BtlBindGfx(1, species, &g_btl_gfx_next);
        return BTL_SLOT_FLAT;
    }

    size = *(signed char *)&g_btl_models[species].pad0A[7];

    switch (size) {
    case 0:
        /* Through a pointer to the wide slot: the original works the array's
           own base back out of it rather than naming it twice. */
        wide = &g_btl_slot_owner[BTL_WIDE0];
        if (*wide == species) {
            found = BTL_SLOT_PAIR;
            goto load;
        }
        {
        u_int slot = BTL_SLOT_FIRST;
        if (*wide == BTL_SLOT_FREE) {
            found = 0xC;
            goto load;
        }
        owner = wide - BTL_WIDE0;
        do {
            if (owner[slot & 0xFFFF] == species) {
                found = slot | BTL_SLOT_NONE;
                goto load;
            }
            if (owner[slot & 0xFFFF] == BTL_SLOT_FREE) {
                found = slot;
            }
            slot += BTL_SLOT_STEP;
        } while ((slot & 0xFFFF) < BTL_SLOT_LAST);
        }
        goto load;

    case 1:
        {
        u_int slot = BTL_SLOT_FIRST;
        do {
            if (g_btl_slot_owner[slot & 0xFFFF] == species) {
                found = slot | BTL_SLOT_NONE;
                goto load;
            }
            if (g_btl_slot_owner[slot & 0xFFFF] == BTL_SLOT_FREE) {
                found = slot;
            }
            slot += BTL_SLOT_STEP;
        } while ((slot & 0xFFFF) < BTL_SLOT_LAST);
        }
        goto load;

    case 2:
        found = BTL_SLOT_FIRST;
        if (g_btl_slot_owner[BTL_WIDE_OWNER] == species) {
            found = BTL_SLOT_WIDE;
            goto load;
        }
        g_btl_slot_owner[BTL_WIDE0] = species;
        g_btl_slot_owner[BTL_WIDE1] = species;
        goto load;

    case 3:
        found = BTL_SLOT_FIRST;
        if (g_btl_slot_owner[BTL_WIDE_OWNER] == species) {
            found = BTL_SLOT_WIDE;
            goto load;
        }
        g_btl_slot_owner[BTL_WIDE0] = species;
        g_btl_slot_owner[BTL_WIDE1] = species;
        g_btl_slot_owner[BTL_WIDE2] = species;
        g_btl_slot_owner[BTL_WIDE3] = species;
        goto load;

    default:
        goto load;
    }

load:
    if ((found & BTL_SLOT_NONE) == 0) {
        /* Nothing owns the slot yet, so the artwork has to go up. */
        u_int slot = found & 0xFFFF;
        g_btl_slot_owner[slot] = species;
        g_btl_slot_owner[slot + 1] = species;
        memcpy(g_btl_gfx_next, image, bytes);
        BtlBindGfx(1, species, &g_btl_gfx_next);
        clut = BtlUploadTim(tim, slot, actor + BTL_ENEMY_SLOT0, 1, 0, 1);
        /* The first copy reads the palette back through a pointer to it. That
           is what keeps the uploaded address in the register the original uses
           across the three copies; it is not redundant. */
        held = &clut;
        g_btl_slot_clut[slot] = clut;
        memcpy(g_btl_enemy_clut + actor * BTL_CLUT_BYTES, (u_char *)*held,
               BTL_CLUT_BYTES);
        memcpy(g_btl_enemy_clut_to + actor * BTL_CLUT_BYTES, (u_char *)clut,
               BTL_CLUT_BYTES);
        memcpy(g_btl_enemy_clut_base + actor * BTL_CLUT_BYTES, (u_char *)clut,
               BTL_CLUT_BYTES);
        g_btl_slot_clut[found & 0xFFFF] =
            (u_long *)(g_btl_enemy_clut + actor * BTL_CLUT_BYTES);
    } else {
        /* This species already holds the slot: take its palette as it stands
           and only ask the GPU where the CLUT ended up. */
        found &= BTL_SLOT_MASK;
        g_btl_clut[actor + BTL_ENEMY_SLOT0] = GetClut(0, actor + BTL_CLUT_Y);
        memcpy(g_btl_enemy_clut + actor * BTL_CLUT_BYTES,
               (u_char *)g_btl_slot_clut[found], BTL_CLUT_BYTES);
        memcpy(g_btl_enemy_clut_to + actor * BTL_CLUT_BYTES,
               (u_char *)g_btl_slot_clut[found], BTL_CLUT_BYTES);
        memcpy(g_btl_enemy_clut_base + actor * BTL_CLUT_BYTES,
               (u_char *)g_btl_slot_clut[found], BTL_CLUT_BYTES);
    }
    return (short)found;
}
#else
INCLUDE_ASM("btlp/nonmatchings/enemyload", BtlLoadEnemyGfx);
#endif

