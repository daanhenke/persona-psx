/* Persona 1 (JP) - giving a combatant its display objects.  BTLP only.
 *   0x80084E90 BtlSpawnActorObj
 *
 * Every actor on the field is two records: the sprite itself and a second one
 * carried on its `attached` link, so the setters in objset.c reach both from
 * the one pointer. The sprite is allocated from a single shared template whose
 * script table is rewritten for the model being spawned - the models differ
 * only in which scripts they run, so one template with the pointer swapped
 * costs nothing and keeps the attribute word in one place.
 *
 * The model's scripts come out of the table at +0x74 of the actor graphics
 * blob, which the overlay's entry point loads once and BtlBindGfx relocates.
 */
#include <types.h>
#include <persona/btlp/object.h>

/* Group 1 is the field: the actors and everything that stands on it. */
#define BTL_ACTOR_GROUP 1

/* Where the graphics blob keeps its per-model table of script tables. */
#define BTL_GFX_SCRIPTS 0x74

/* The shadow's entry in its own template table. */
#define BTL_SHADOW_INDEX 0x13

extern BtlObjDef        g_btl_actor_def;
extern const BtlObjDef  g_btl_shadow_defs[];
extern u_char          *g_btl_actor_gfx;

extern BtlObj *BtlObjAlloc(const BtlObjDef *defs, int group, BtlObj *after,
                           int a3, int index, const long *pos, int p7, int p8);

BtlObj *BtlSpawnActorObj(int model, const long *pos)
{
    BtlObj *obj;

    g_btl_actor_def.scripts =
        ((const u_long ***)(g_btl_actor_gfx + BTL_GFX_SCRIPTS))[model];
    obj = BtlObjAlloc(&g_btl_actor_def, BTL_ACTOR_GROUP, 0, 5, 0, pos,
                      0x18, model + 0x40);
    obj->attached = BtlObjAlloc(g_btl_shadow_defs, BTL_ACTOR_GROUP, 0, 5,
                                BTL_SHADOW_INDEX, pos, 0x1F, 0x20);
    return obj;
}
