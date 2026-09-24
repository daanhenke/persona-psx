/* Persona 1 (JP) - giving an enemy its display objects.  BTLP only.
 *   0x80083A70 BtlSpawnEnemy
 *
 * The enemy twin of BtlSpawnActorObj: one template, allocated from three times.
 * The body comes first, then a shadow chained behind it and hung on the body's
 * `shadow` link, and then - where the species record names one - a third piece
 * on `attached`. All three take the position the column and row work out to,
 * and carry the species number and the grid square in the bytes above `kind`.
 *
 * Which scripts a body runs is the species record's `spawn` entry indexed into
 * that species' graphics blob. Four species are reached differently: 0xB3 and
 * the pair 0xB6..0xB7 keep their table at a fixed offset in the blob, and 0xB4
 * takes the first table of its own.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>

/* Group 4 is the enemy side. */
#define BTL_ENEMY_GROUP 4

/* Where the grid puts a square, in whole pixels before the 16.16 shift. */
#define BTL_GRID_X    15
#define BTL_GRID_Y    20
#define BTL_GRID_LEFT (-60)
#define BTL_GRID_TOP  (-140)

/* The species whose script tables are not reached through `spawn`. */
#define BTL_SPECIES_ODD   0xB3
#define BTL_SPECIES_PLAIN 0xB4
#define BTL_SPECIES_PAIR0 0xB5
#define BTL_SPECIES_PAIR1 0xB8
#define BTL_ODD_SCRIPTS   0x60
#define BTL_PAIR_SCRIPTS  0x40

/* What each of the three records starts with on top of its template. */
#define BTL_BODY_ATTR   0x200
#define BTL_SHADOW_ATTR 0xC
#define BTL_EXTRA_ATTR  0x40000041

/* The shadow sits flat, a little below the body. */
#define BTL_SHADOW_SCALE 0xC00
#define BTL_SHADOW_DROP  (-0x320)

/* BtlObjAlloc's last argument, and how far the caller's depth is pushed. */
#define BTL_SHADOW_SLOT 0x1F
#define BTL_DEPTH_STEP  5

extern BtlObjDef         g_btl_enemy_def;
extern u_char           *g_btl_species_gfx[];

/* 97.85%. The species pick is a switch whose every arm stores the scripts
   itself. The arms' stores are merged after scheduling, so each is
   scheduled in its own case, which the image shows. The shadow takes its
   scripts and attribute before the rest. What is left is one load: the
   image reads the body's scripts before storing its kind, column and row,
   and stores the scripts after them. Written first, the store and the
   attribute come up with the load (93.55%). Written last, as here, the load
   waits behind the three stores. */
#ifdef NON_MATCHING
BtlObj *BtlSpawnEnemy(int species, int col, int row, short gfx, int depth)
{
    BtlObj *obj;
    BtlObj *shadow;
    long    pos[3];
    int     slot;

    g_btl_enemy_def.attr = 0;
    switch (species) {
    case BTL_SPECIES_PLAIN:
        g_btl_enemy_def.scripts = *(const u_long ***)g_btl_species_gfx[BTL_SPECIES_PLAIN];
        break;
    case BTL_SPECIES_ODD:
        g_btl_enemy_def.scripts = *(const u_long ***)
            (g_btl_species_gfx[BTL_SPECIES_ODD] + BTL_ODD_SCRIPTS);
        break;
    case BTL_SPECIES_PAIR0 + 1:
    case BTL_SPECIES_PAIR0 + 2:
        g_btl_enemy_def.scripts = *(const u_long ***)
            (g_btl_species_gfx[species] + BTL_PAIR_SCRIPTS);
        break;
    default:
        g_btl_enemy_def.scripts = *(const u_long ***)
            (g_btl_species_gfx[species] + g_btl_models[species].spawn * 4);
        break;
    }
    pos[0] = (col * BTL_GRID_X + BTL_GRID_LEFT) << 16;
    pos[1] = (row * BTL_GRID_Y + BTL_GRID_TOP) << 16;
    slot = depth + BTL_DEPTH_STEP;
    pos[2] = (int)g_btl_models[species].depth << 16;

    obj = BtlObjAlloc(&g_btl_enemy_def, BTL_ENEMY_GROUP, 0, 5, 0, pos,
                      gfx, slot);
    obj->kind = species;
    obj->col2 = col;
    obj->row = row;
    obj->scripts = (const u_long **)g_btl_species_gfx[species];
    obj->attr |= BTL_BODY_ATTR | g_btl_species[species].attr;

    pos[2] = 0;
    shadow = BtlObjAlloc(&g_btl_enemy_def, BTL_ENEMY_GROUP, obj, 5, 0, pos,
                         gfx, BTL_SHADOW_SLOT);
    shadow->scripts = (const u_long **)g_btl_species_gfx[species];
    shadow->scale_y = BTL_SHADOW_SCALE;
    shadow->kind = species;
    shadow->col2 = col;
    shadow->row = row;
    shadow->rot.vx = BTL_SHADOW_DROP;
    shadow->rot.vy = 0;
    shadow->rot.vz = 0;
    shadow->attr |= BTL_SHADOW_ATTR;
    obj->shadow = shadow;

    if (g_btl_models[species].extra != 0) {
        g_btl_enemy_def.scripts = *(const u_long ***)
            (g_btl_models[species].extra * 4
             + (int)g_btl_species_gfx[species]);
        shadow = BtlObjAlloc(&g_btl_enemy_def, BTL_ENEMY_GROUP, shadow, 5, 0,
                             pos, gfx, slot);
        shadow->attr |= BTL_EXTRA_ATTR;
        obj->attached = shadow;
    }
    return obj;
}
#else
INCLUDE_ASM("btlp/nonmatchings/enemyobj", BtlSpawnEnemy);
#endif

