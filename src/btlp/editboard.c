/* Persona 1 (JP) - the debug character editor's board.  BTLP only.
 *   0x800A8A44 BtlOpenEditBoard  0x800A8BA8 BtlShutEditBoard
 *   0x800A8C00 BtlFillEditBoard
 *
 * The editor lets the level and the stats of one fighter be stepped up and
 * down with the shoulder buttons, and puts everything that follows from them
 * up on a board of its own.
 *
 * The board is built the way BtlBoardOpen builds one, only from seven parts
 * rather than four: each part is chained to the one before through
 * BtlObj.attached, the first six trail, and the last - which is what
 * g_btl_edit_board keeps - is started growing into place. The first part's
 * picture is the one flag D_800CCA24 picks between, the same flag
 * BtlBoardOpen reads. Shutting it sets the last part shrinking away again;
 * nothing is freed.
 *
 * Filling it writes the fighter's name, level and every number on the sheet
 * into the cells the board draws them from, and lays out three bars per stat
 * end to end: the stat as the character has it, what the equipment adds on
 * top - cut short at 99 - and whatever the Persona adds past that.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/board.h>
#include <persona/btlp/number.h>
#include <persona/btlp/object.h>

/* The two pictures the first part is drawn from. */
#define EDIT_PICTURE     10
#define EDIT_PICTURE_ALT 9

#define EDIT_PARTS 7
#define EDIT_GROUP 1

/* Where the board stands, and how it is started. */
#define EDIT_X 0xA00000
#define EDIT_Y 0x980000

#define EDIT_SCALE     0x20
#define EDIT_SCALE_Z   0x1000
#define EDIT_OPENING   3
#define EDIT_CLOSING   4

/* One bar of the sheet: a gradient quad, placed and sized by the fill. Every
   bar starts at EDIT_BAR_X on disc; the fill moves the second and third of a
   stat along to where the one before it ends. */
typedef struct {
    /* 0x00 */ short   x;
    /* 0x02 */ short   y;
    /* 0x04 */ u_short w;
    /* 0x06 */ u_short h;
    /* 0x08 */ u_char  rgb[6][4];
    /* 0x20 */ u_char  unk20[0x18];
} BtlEditBar;                          /* 0x38 bytes */

#define EDIT_BAR_X  (-0x40)
#define EDIT_STAT_MAX 99

/* The fighter's name, copied whole. */
typedef struct {
    u_char b[10];
} BtlEditName;

extern const BtlObjDef g_btl_shadow_defs[];
extern const BtlObjDef g_btl_lone_defs[];
extern const BtlObjDef D_800DF88C[];
extern const BtlObjDef D_800DF8D4[];

extern u_char     D_800CCA24;
extern BtlObj    *g_btl_edit_board;
extern BtlEditBar g_btl_edit_bars[];

/* Where the board reads everything it draws: the name, and a run of digit
   cells per number, each named for the field it shows. */
extern BtlEditName g_btl_edit_name;
extern u_char g_btl_edit_level_cells[];
extern u_char g_btl_edit_unk56_cells[];
extern u_char g_btl_edit_hp_cells[];
extern u_char g_btl_edit_hp_max_cells[];
extern u_char g_btl_edit_sp_cells[];
extern u_char g_btl_edit_sp_max_cells[];
extern u_char g_btl_edit_unk1C_cells[];
extern u_char g_btl_edit_unk10_cells[];
extern u_char g_btl_edit_unk18_cells[];
extern u_char g_btl_edit_melee_atk_cells[];
extern u_char g_btl_edit_melee_hit_cells[];
extern u_char g_btl_edit_gun_atk_cells[];
extern u_char g_btl_edit_gun_hit_cells[];
extern u_char g_btl_edit_defence_cells[];
extern u_char g_btl_edit_unk3A_cells[];
extern u_char g_btl_edit_unk3C_cells[];
extern u_char g_btl_edit_evade_cells[];
extern u_char g_btl_edit_strength_cells[];
extern u_char g_btl_edit_vitality_cells[];
extern u_char g_btl_edit_dexterity_cells[];
extern u_char g_btl_edit_agility_cells[];
extern u_char g_btl_edit_luck_cells[];

BtlBoardDef g_btl_edit_board_parts[EDIT_PARTS] = {
    { g_btl_shadow_defs, 9, 1, 0x0D, 0x1D },
    { g_btl_lone_defs, 0x19, 3, 0x1F, 0x20 },
    { g_btl_lone_defs, 0x1A, 1, 0x1F, 0x20 },
    { g_btl_lone_defs, 0x1B, 1, 0x1F, 0x27 },
    { g_btl_lone_defs, 0x1C, 9, 0x1F, 0x27 },
    { D_800DF88C, 0x05, 1, 0x1F, 0x25 },
    { D_800DF8D4, 0x05, 7, 0x02, 0x02 },
};

BtlObj *BtlOpenEditBoard(void)
{
    BtlBoardDef *part;
    BtlObj      *o;
    BtlObj      *prev;
    long         pos[3];
    int          picture;
    int          i;

    part = g_btl_edit_board_parts;
    picture = EDIT_PICTURE;
    pos[0] = EDIT_X;
    pos[1] = EDIT_Y;
    pos[2] = 0;
    if (D_800CCA24 != 0) {
        picture = EDIT_PICTURE_ALT;
    }
    g_btl_edit_board_parts[0].index = picture;

    i = 0;
    prev = NULL;
    do {
        o = BtlObjAlloc(part->defs, EDIT_GROUP, prev, part->kind, part->index,
                        pos, part->p7, part->p8);
        i++;
        part++;
        o->attached = prev;
        o->attr |= BTL_OBJ_TRAIL;
        prev = o;
    } while (i < EDIT_PARTS - 1);

    o = BtlObjAlloc(part->defs, EDIT_GROUP, o, part->kind, part->index, pos,
                    part->p7, part->p8);
    o->attached = prev;
    o->attr |= 0x1000;
    BtlObjSetScale(o, EDIT_SCALE, EDIT_SCALE, EDIT_SCALE_Z);
    BtlObjSetMotion(o, EDIT_OPENING);
    return g_btl_edit_board = o;
}

void BtlShutEditBoard(void)
{
    BtlObjSetTimer(g_btl_edit_board, 0);
    BtlObjSetPhase(g_btl_edit_board, 1);
    BtlObjSetScaleTo(g_btl_edit_board, EDIT_SCALE);
    BtlObjSetMotion(g_btl_edit_board, EDIT_CLOSING);
}

/* The equipment's bar: its share, cut short where the stat and it would pass
   99, and nothing at all once the stat is there already. */
#define EDIT_EQUIP_W(base, equip)                                              \
    ((base) < EDIT_STAT_MAX                                                    \
         ? ((base) + (equip) < EDIT_STAT_MAX + 1 ? (equip) * 2                 \
                                                 : (EDIT_STAT_MAX - (base)) * 2) \
         : 0)

void BtlFillEditBoard(BtlActor *a)
{
    int w0, w1, w2, w3, w4;

    g_btl_edit_name = *(BtlEditName *)a->c.name;

    BtlDrawNumber(g_btl_edit_level_cells, a->c.level, 2);
    BtlDrawNumber(g_btl_edit_unk56_cells, a->c.unk56, 2);
    BtlDrawNumberAlt(g_btl_edit_hp_cells, a->c.hp, 3);
    BtlDrawNumberAlt(g_btl_edit_hp_max_cells, a->c.hp_max, 3);
    BtlDrawNumberAlt(g_btl_edit_sp_cells, a->c.sp, 3);
    BtlDrawNumberAlt(g_btl_edit_sp_max_cells, a->c.sp_max, 3);
    BtlDrawNumber(g_btl_edit_unk1C_cells, a->c.unk1C, 7);
    BtlDrawNumber(g_btl_edit_unk10_cells, a->c.unk10, 7);
    BtlDrawNumber(g_btl_edit_unk18_cells, a->c.unk18, 7);
    BtlDrawNumber(g_btl_edit_melee_atk_cells, a->c.melee_atk, 3);
    BtlDrawNumber(g_btl_edit_melee_hit_cells, a->c.melee_hit, 3);
    BtlDrawNumber(g_btl_edit_gun_atk_cells, a->c.gun_atk, 3);
    BtlDrawNumber(g_btl_edit_gun_hit_cells, a->c.gun_hit, 3);
    BtlDrawNumber(g_btl_edit_defence_cells, a->c.defence, 3);
    BtlDrawNumber(g_btl_edit_unk3A_cells, a->c.unk3A, 3);
    BtlDrawNumber(g_btl_edit_unk3C_cells, a->c.unk3C, 3);
    BtlDrawNumber(g_btl_edit_evade_cells, a->c.evade, 3);
    BtlDrawNumber(g_btl_edit_strength_cells, a->c.stat[0], 2);
    BtlDrawNumber(g_btl_edit_vitality_cells, a->c.stat[1], 2);
    BtlDrawNumber(g_btl_edit_dexterity_cells, a->c.stat[2], 2);
    BtlDrawNumber(g_btl_edit_agility_cells, a->c.stat[3], 2);
    BtlDrawNumber(g_btl_edit_luck_cells, a->c.stat[4], 2);

    g_btl_edit_bars[0].w = w0 = a->c.stat_base[0] * 2;
    g_btl_edit_bars[1].w = w1 = a->c.stat_base[1] * 2;
    g_btl_edit_bars[2].w = w2 = a->c.stat_base[2] * 2;
    g_btl_edit_bars[3].w = w3 = a->c.stat_base[3] * 2;
    g_btl_edit_bars[4].w = w4 = a->c.stat_base[4] * 2;
    g_btl_edit_bars[5].x = w0 + EDIT_BAR_X;
    g_btl_edit_bars[6].x = w1 + EDIT_BAR_X;
    g_btl_edit_bars[7].x = w2 + EDIT_BAR_X;
    g_btl_edit_bars[8].x = w3 + EDIT_BAR_X;
    g_btl_edit_bars[9].x = w4 + EDIT_BAR_X;

    g_btl_edit_bars[5].w = EDIT_EQUIP_W(a->c.stat_base[0], a->equip_stat[0]);
    g_btl_edit_bars[6].w = EDIT_EQUIP_W(a->c.stat_base[1], a->equip_stat[1]);
    g_btl_edit_bars[7].w = EDIT_EQUIP_W(a->c.stat_base[2], a->equip_stat[2]);
    g_btl_edit_bars[8].w = EDIT_EQUIP_W(a->c.stat_base[3], a->equip_stat[3]);
    g_btl_edit_bars[9].w = EDIT_EQUIP_W(a->c.stat_base[4], a->equip_stat[4]);

    g_btl_edit_bars[10].x =
        g_btl_edit_bars[0].w + g_btl_edit_bars[5].w + EDIT_BAR_X;
    g_btl_edit_bars[11].x =
        g_btl_edit_bars[1].w + g_btl_edit_bars[6].w + EDIT_BAR_X;
    g_btl_edit_bars[12].x =
        g_btl_edit_bars[2].w + g_btl_edit_bars[7].w + EDIT_BAR_X;
    g_btl_edit_bars[13].x =
        g_btl_edit_bars[3].w + g_btl_edit_bars[8].w + EDIT_BAR_X;
    g_btl_edit_bars[14].x =
        g_btl_edit_bars[4].w + g_btl_edit_bars[9].w + EDIT_BAR_X;

    g_btl_edit_bars[10].w =
        (a->c.stat[0] - (a->c.stat_base[0] + a->equip_stat[0])) * 2;
    g_btl_edit_bars[11].w =
        (a->c.stat[1] - (a->c.stat_base[1] + a->equip_stat[1])) * 2;
    g_btl_edit_bars[12].w =
        (a->c.stat[2] - (a->c.stat_base[2] + a->equip_stat[2])) * 2;
    g_btl_edit_bars[13].w =
        (a->c.stat[3] - (a->c.stat_base[3] + a->equip_stat[3])) * 2;
    g_btl_edit_bars[14].w =
        (a->c.stat[4] - (a->c.stat_base[4] + a->equip_stat[4])) * 2;
}
