/* Persona 1 (JP) - putting a party member on the field.  BTLP only.
 *   0x8008445C BtlSpawnMemberObj
 *
 * The party's counterpart to BtlSpawnActorObj, and it works the same way: one
 * shared template whose script table is rewritten for the model going out,
 * then a second record for the shadow hung off the first.
 *
 * Which script the member stands in is two lookups deep - g_btl_member_scripts
 * gives an index by model and by the actor's script_pick, and that indexes the
 * model's own script table - so the same pose reads differently per character.
 *
 * The position comes off the formation grid rather than being passed in:
 * fifteen pixels a column and twenty a row, both in 16.16, which is what puts
 * the party in its wedge.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* The group the party and the enemies share. */
#define BTL_FIELD_GROUP 5

/* Where the grid starts, and how far apart its cells are on screen. */
#define MEMBER_X_STEP 15
#define MEMBER_X_BASE (-0x3C)
#define MEMBER_Y_STEP 20
#define MEMBER_Y_BASE 0x3C

/* Stride of g_btl_member_scripts, by model and by script_pick. */
#define MEMBER_SCRIPT_MODEL 0x28
#define MEMBER_SCRIPT_PICK  10

/* The shadow: three quarters of the height, and lifted. */
#define SHADOW_SCALE_Y 0xC00
#define SHADOW_LIFT    (-0x320)
#define SHADOW_ATTR    0xC

extern BtlObjDef     g_btl_member_def;
extern u_char       *g_btl_member_gfx[];
extern const u_char  g_btl_member_scripts[];

extern BtlObj *BtlObjAlloc(const BtlObjDef *defs, int group, BtlObj *after,
                           int a3, int index, const long *pos, int p7, int p8);

BtlObj *BtlSpawnMemberObj(int model, int col, int row, short p7, int slot)
{
    BtlObj        *obj;
    BtlObj        *shadow;
    const u_char  *scripts;
    u_char       **gfx;
    u_char        *table;
    long           pos[3];

    /* Both tables are indexed by the model, and the original works the two
       bases out together before it looks anything up. */
    scripts = &g_btl_member_scripts[model * MEMBER_SCRIPT_MODEL];
    gfx = &g_btl_member_gfx[model];
    g_btl_member_def.attr = 0;
    g_btl_member_def.scripts =
        ((const u_long ***)*gfx)
            [scripts[g_btl_actors[slot].script_pick * MEMBER_SCRIPT_PICK]];

    pos[0] = (col * MEMBER_X_STEP + MEMBER_X_BASE) << 16;
    pos[1] = (row * MEMBER_Y_STEP + MEMBER_Y_BASE) << 16;
    pos[2] = 0;

    obj = BtlObjAlloc(&g_btl_member_def, BTL_FIELD_GROUP, 0, 5, 0, pos,
                      p7, slot);
    table = *gfx;
    obj->kind = model;
    obj->col2 = col;
    obj->row = row;
    obj->scripts = (const u_long **)table;

    shadow = BtlObjAlloc(&g_btl_member_def, BTL_FIELD_GROUP, obj, 5, 0, pos,
                         p7, 0x1F);
    shadow->scale_y = SHADOW_SCALE_Y;
    shadow->kind = model;
    shadow->col2 = col;
    shadow->row = row;
    shadow->unk70 = SHADOW_LIFT;
    shadow->unk72 = 0;
    shadow->unk74 = 0;
    shadow->attr |= SHADOW_ATTR;
    obj->shadow = shadow;
    return obj;
}
