/* Persona 1 (JP) - fusion accidents and bonuses.  ADV only.
 *   0x800A0450 FuseApplyBonus     0x800A0744 FuseRollKind
 *   0x800A080C FuseRollAccident   0x800A08E4 FuseItemsDraw
 *   0x800A0BB8 FuseItemBonus
 *
 * The rolls behind a fusion (fusion.c): whether an accident strikes and
 * what it does, which kind a stray result comes out as, the bonus a fusion
 * of a given kind leaves on the new Persona, and what an offered item adds.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/persona.h>
#include <persona/common/item.h>
#include <persona/adv/personapage.h>

#define ITEM_ID      0x1FF
#define GLYPH_DIGIT0 0xC0

typedef struct {
    u_short arcana;
    u_short unk2;
    u_short unk4;
    u_short persona;
    u_short flag;
} FuseResult;

#define g_fuse      (*(FuseResult *)0x801F1B8C)
#define g_item_list ((u_short *)0x800EAE4C)

/* What an offered item adds to a fusion. */
typedef struct {
    short mag;
    short stat[PERSONA_STATS];
    short unkC;
    short random;
    short persona;
} FuseBonus;

extern short   D_800BB7F4;
extern u_char  g_moon;
extern u_char  g_kind_labels[];
extern u_char  D_800BA0E4[];
extern u_char  D_800B9CAB[][0x16];

extern int   rand(void);
extern short func_800AB23C(short persona, u_char src, void *spell, void *rank);
extern void  DrawItemName(int id, short *dst, u_short base, int b);
extern void  func_800A1990(u_char a, u_char b, short mode, FuseResult *out,
                           short special);

/* 96.7%: the image loads both stock bytes before converting `mode` and
   keeps them in a3/a1; this build converts first. */
#ifdef NON_MATCHING
/* A fusion of kind 1 raises magic attack by 20 and a stat by 5; kinds 1 and
   3 of the second roll carry an affinity across from a source and move every
   stat a point up or down, kind 2 only the affinity. */
void FuseApplyBonus(short p, short kind, short mode)
{
    Persona *r = &g_personas[p];
    int      s;
    u_char   a;
    u_char   b;
    u_char   spell;
    short    rank;
    int      top;

    if (kind == 1) {
        s = g_personas[p].stat[0];
        top = s < g_personas[p].stat[1];
        g_personas[p].mag_atk += 20;
        if (s < g_personas[p].stat[2]) {
            top = 2;
        }
        if (s < g_personas[p].stat[3]) {
            top = 3;
        }
        if (s < g_personas[p].stat[4]) {
            top = 4;
        }
        switch (top) {
        case 0:
            r->stat[0] += 5;
            break;
        case 1:
            r->stat[1] += 5;
            break;
        case 2:
            r->stat[2] += 5;
            break;
        case 3:
            r->stat[3] += 5;
            break;
        case 4:
            r->stat[4] += 5;
            break;
        }
    }
    a = g_persona_stock[g_menu->top.cur];
    b = g_persona_stock[g_menu->status_page.cur];
    switch (mode) {
    case 1:
        if (func_800AB23C(g_fuse.persona, a, &spell, &rank) && r->raw[6] == 0) {
            r->raw[6] = spell;
        }
        r->stat[0] += 1;
        r->stat[1] += 1;
        r->stat[2] += 1;
        r->stat[3] += 1;
        r->stat[4] += 1;
        break;
    case 2:
        if (func_800AB23C(g_fuse.persona, b, &spell, &rank) && r->raw[6] == 0) {
            r->raw[6] = spell;
        }
        break;
    case 3:
        if (func_800AB23C(g_fuse.persona, a, &spell, &rank) && r->raw[6] == 0) {
            r->raw[6] = spell;
        }
        r->stat[0] -= 1;
        r->stat[1] -= 1;
        r->stat[2] -= 1;
        r->stat[3] -= 1;
        r->stat[4] -= 1;
        break;
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/fuserules", FuseApplyBonus);
#endif

/* The kind a stray result comes out as. */
short FuseRollKind(void)
{
    int r = rand() & 0xFF;

    if (r < 0x20) {
        return 9;
    }
    if (r < 0x40) {
        return 0x13;
    }
    if (r < 0x60) {
        return 0x15;
    }
    if (r < 0x80) {
        return 6;
    }
    if (r < 0xA0) {
        return 0xE;
    }
    if (r < 0xC0) {
        return 0x12;
    }
    if (r < 0xCA) {
        return 1;
    }
    if (r < 0xD0) {
        return 2;
    }
    if (r < 0xDA) {
        return 4;
    }
    if (r < 0xE0) {
        return 5;
    }
    if (r < 0xEA) {
        return 0xB;
    }
    if (r < 0xF0) {
        return 0x14;
    }
    if (r < 0xFA) {
        return 3;
    }
    return 7;
}

/* Which accident strikes, by the result's rank: 4 a stray kind, 3 an item
   roll, 2 five points down, 1 two points up, 0 an affinity. */
short FuseRollAccident(short rank)
{
    int r = rand() & 0xFF;

    switch (rank) {
    case 0:
        if (r < 0x40) {
            return 4;
        }
        if (r < 0x48) {
            return 3;
        }
        if (r < 0x80) {
            return 2;
        }
        return r < 0xC0;
    case 1:
        if (r < 0x20) {
            return 4;
        }
        if (r < 0x40) {
            return 3;
        }
        return r < 0xC0;
    case 2:
        if (r < 0x20) {
            return 4;
        }
        if (r < 0x30) {
            return 3;
        }
        return (r < 0xB0) * 2;
    }
}

/* 96.6%: the image schedules the result pointer's setup a few slots
   later in the prologue. */
#ifdef NON_MATCHING
/* The offering line: the two sources, the item, and what they make. */
void FuseItemsDraw(void)
{
    u_short *res;
    short    n;

    n = D_800BB7F4 * 2 + g_menu->unk2E0.cur + g_menu->unk2D0.cur * 2;
    res = &g_fuse.persona;
    TileMapFillRect(AT(g_tilemap2, 1, 2), 0, 0xB, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap2, 1, 16), 0, 0xB, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap2, 3, 10), 0, 0xA, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap2, 5, 1), 0, 0xA, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap2, 5, 14), 0, 2, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap2, 5, 17), 0, 0xA, 1, MAP_W);
    *AT(g_tilemap2, 1, 1) = g_menu->status_page.cur + 0x418;
    DrawPersonaName(g_persona_stock[g_menu->status_page.cur], AT(g_tilemap2, 1, 2),
                    0);
    *AT(g_tilemap2, 1, 15) = g_menu->top.cur + 0x418;
    DrawPersonaName(g_persona_stock[g_menu->top.cur], AT(g_tilemap2, 1, 16), 0);
    DrawItemName(g_item_list[n] & ITEM_ID, AT(g_tilemap2, 3, 10), 0, 0);
    func_800A1990(g_persona_stock[g_menu->status_page.cur],
                  g_persona_stock[g_menu->top.cur], g_item_list[n] & ITEM_ID,
                  &g_fuse, 0);
    TileMapWriteRow(&g_kind_labels[(short)(g_persona_defs[*res].kind - 1) * 10],
                    AT(g_tilemap2, 5, 1), 0, 10);
    TileMapWriteRow(g_persona_defs[*res].name, AT(g_tilemap2, 5, 17), 0, 10);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 5, 15), GLYPH_DIGIT0,
        FormatDecimal(g_persona_defs[*res].level, g_hud_digits, 2));
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/fuserules", FuseItemsDraw);
#endif

/* 97.5%: the arcana table's base is added as a register in the image
   rather than folded into the load. */
#ifdef NON_MATCHING
/* An item's contribution: by its class, magic attack, a stat, the arcana's
   own value, a fixed Persona, or - for the random class - a stat nudged up
   or down or, one time in five, a Persona picked by the moon. */
void FuseItemBonus(short arcana, short item, int unused2, int unused3,
                   FuseBonus *out)
{
    int    r;
    u_char f;

    out->mag = 0;
    out->stat[0] = 0;
    out->stat[1] = 0;
    out->stat[2] = 0;
    out->stat[3] = 0;
    out->stat[4] = 0;
    out->random = 0;
    out->persona = 0;
    out->unkC = 0;
    if (item == 0) {
        return;
    }
    f = g_item_defs[item].pad1D[0];
    switch (f & 0xE0) {
    case 0xE0:
        r = rand() & 0xFF;
        if (r >= 0xC9) {
            r = D_800BA0E4[0x230 + (g_moon & 0xF)];
            out->persona = D_800BA0E4[(rand() & 0xFF) / 16 * 9 + r + 0x48C];
        } else {
            switch (r / 8) {
            case 0:
            case 1:
            case 2:
                out->stat[0] = 3;
                break;
            case 3:
            case 4:
            case 5:
                out->stat[1] = 3;
                break;
            case 6:
            case 7:
            case 8:
                out->stat[2] = 3;
                break;
            case 9:
            case 10:
            case 11:
                out->stat[3] = 3;
                break;
            case 12:
            case 13:
            case 14:
                out->stat[4] = 3;
                break;
            case 15:
            case 16:
                out->stat[0] = -3;
                break;
            case 17:
            case 18:
                out->stat[1] = -3;
                break;
            case 19:
            case 20:
                out->stat[2] = -3;
                break;
            case 21:
            case 22:
                out->stat[3] = -3;
                break;
            case 23:
            case 24:
                out->stat[4] = -3;
                break;
            }
        }
        out->random = 1;
        break;
    case 0xA0:
        out->unkC = D_800B9CAB[item][arcana];
        break;
    case 0x60:
        switch (f & 0x1F) {
        case 16:
            r = 1;
            break;
        case 8:
            r = 2;
            break;
        case 4:
            r = 3;
            break;
        case 2:
            r = 4;
            break;
        case 1:
            r = 5;
            break;
        }
        ((short *)out)[r] = g_item_defs[item].pad1D[1];
        break;
    case 0x20:
        out->mag = g_item_defs[item].pad1D[1];
        break;
    case 0:
        out->persona = g_item_defs[item].pad1D[1];
        break;
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/fuserules", FuseItemBonus);
#endif
