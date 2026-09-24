/* Persona 1 (JP) - filling in the enemy status board.  BTLP only.
 *   0x800ABC00 BtlShowEnemyStatus
 *
 * The board an enemy's own numbers are shown on. Its name row is always
 * filled: the arcana's label and then the demon's name after it. Everything
 * below is only shown for a demon the party has analysed - the bit for its key
 * in g_analysed_demons - or with the debug switch beside the rest of them
 * raised; otherwise every row takes the placeholder glyphs instead.
 *
 * The fighter's derived numbers are worked out again first, so the board shows
 * what the demon would fight with rather than what its record came off the
 * disc holding.
 *
 * The label the kind row takes is picked out of a two-digit code on the
 * demon's own definition: the tens choose which run of the table to read and
 * the units how far into it, with the first two runs sharing one.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/number.h>
#include <persona/btlp/stats.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>
#include <persona/common/spell.h>

/* Cells a row of each kind holds, and the byte that ends one. */
#define STATUS_NAME_CELLS  8
#define STATUS_SPELL_ROW   11
#define STATUS_SPELL_ROWS  6
#define STATUS_RESIST_ROW  0x19
#define STATUS_END         0xFF

/* How wide each figure is drawn. */
#define STATUS_DIGITS_2 2
#define STATUS_DIGITS_3 3

/* The kind code's two digits, and where each run of labels starts. */
#define STATUS_KIND_BASE 10
#define STATUS_KIND_RUN2 4
#define STATUS_KIND_RUN3 8
#define STATUS_KIND_RUN4 0xC

/* One row of each label table. */
#define STATUS_RANK_ROW 10
#define STATUS_KIND_ROW 11
#define STATUS_AIL_ROW  6

/* The placeholder run as a row takes it, two or three cells at a time. */
typedef struct {
    signed char c[3];
} BtlStatusBlank3;
typedef struct {
    signed char c[2];
} BtlStatusBlank2;

#define BtlStatusBlank(n, row)                               \
    (*(BtlStatusBlank##n *)&(row) =                          \
         *(BtlStatusBlank##n *)&g_btl_status_unknown_cells)

/* The labels a demon answers each element with, a row of cells per
   Char.resist. */
extern u_char g_resist_labels[][STATUS_RESIST_ROW];

void BtlShowEnemyStatus(int slot)
{
    BtlActor    *a;
    PersonaData *def;
    u_char      *cell;
    u_char      *src;
    u_int        key;
    int          i;
    int          kind;
    int          code;
    int          n;
    /* The byte the spell rows are cleared with, held rather than written out
       at the store: the image settles it before the row counter, which is
       where the two constants' order comes from. */
    int          end;

    i = 0;
    a = &g_btl_combatants[slot];
    def = &g_persona_data[a->c.key];
    BtlDeriveBattleStats(a);
    cell = g_btl_status_name_cells;
    src = g_btl_arcana_labels[a->species];
    for (i = 0; i < STATUS_NAME_CELLS; i++) {
        if (*src == STATUS_END) {
            break;
        }
        *cell = *src;
        src++;
        cell++;
    }
    memcpy(cell, a->c.name, CHAR_NAME_CELLS);
    memcpy(g_btl_status_ail_cells,
           g_btl_status_labels[(signed char)a->c.status], STATUS_AIL_ROW);
    BtlDrawNumber((u_char *)&g_btl_status_level_cells, a->c.level,
                  STATUS_DIGITS_2);

    if (g_btl_debug_show_stats != 0
        || (key = a->c.key,
            (g_analysed_demons[key >> 5] & (1 << (key & 0x1F))) != 0)) {
        memcpy(&g_btl_status_resist_cells, g_resist_labels[a->c.resist],
               STATUS_RESIST_ROW);
        BtlDrawNumberAlt((u_char *)&g_btl_status_hp_cells, a->c.hp,
                         STATUS_DIGITS_3);
        BtlDrawNumberAlt((u_char *)&g_btl_status_hp_max_cells, a->c.hp_max,
                         STATUS_DIGITS_3);
        BtlDrawNumberAlt((u_char *)&g_btl_status_sp_cells, a->c.sp,
                         STATUS_DIGITS_3);
        BtlDrawNumberAlt((u_char *)&g_btl_status_sp_max_cells, a->c.sp_max,
                         STATUS_DIGITS_3);
        BtlDrawNumber((u_char *)&g_btl_status_strength_cells,
                      a->stat[STAT_STRENGTH], STATUS_DIGITS_2);
        BtlDrawNumber((u_char *)&g_btl_status_vitality_cells,
                      a->stat[STAT_VITALITY], STATUS_DIGITS_2);
        BtlDrawNumber((u_char *)&g_btl_status_dexterity_cells,
                      a->stat[STAT_DEXTERITY], STATUS_DIGITS_2);
        BtlDrawNumber((u_char *)&g_btl_status_agility_cells,
                      a->stat[STAT_AGILITY], STATUS_DIGITS_2);
        BtlDrawNumber((u_char *)&g_btl_status_luck_cells, a->stat[STAT_LUCK],
                      STATUS_DIGITS_2);
        BtlDrawNumber((u_char *)&g_btl_status_mag_atk_cells, a->mag_atk,
                      STATUS_DIGITS_3);
        BtlDrawNumber((u_char *)&g_btl_status_mag_def_cells, a->mag_def,
                      STATUS_DIGITS_3);
        memcpy(&g_btl_status_rank_cells,
               &g_btl_status_rank_labels[a->persona_rank * STATUS_RANK_ROW],
               STATUS_RANK_ROW);
        code = g_persona_data[a->c.key].pad36[0];
        kind = code / STATUS_KIND_BASE;
        switch (kind) {
        case 0:
        case 1:
            i = code % STATUS_KIND_BASE;
            break;
        case 2:
            i = code % STATUS_KIND_BASE + STATUS_KIND_RUN2;
            break;
        case 3:
            i = code % STATUS_KIND_BASE + STATUS_KIND_RUN3;
            break;
        case 4:
            i = code % STATUS_KIND_BASE + STATUS_KIND_RUN4;
            break;
        }
        memcpy(&g_btl_status_kind_cells,
               &g_btl_status_kind_labels[i * STATUS_KIND_ROW],
               STATUS_KIND_ROW - 1);
        i = 0;
        do {
            memcpy(&g_btl_status_spell_cells[i * STATUS_SPELL_ROW],
                   g_spell_data[def->spell[i]].name, SPELL_NAME_CELLS);
            i++;
        } while (i < STATUS_SPELL_ROWS);
        return;
    }

    BtlStatusBlank(3, g_btl_status_resist_cells);
    BtlStatusBlank(3, g_btl_status_hp_cells);
    BtlStatusBlank(3, g_btl_status_hp_max_cells);
    BtlStatusBlank(3, g_btl_status_sp_cells);
    BtlStatusBlank(3, g_btl_status_sp_max_cells);
    BtlStatusBlank(2, g_btl_status_strength_cells);
    BtlStatusBlank(2, g_btl_status_vitality_cells);
    BtlStatusBlank(2, g_btl_status_dexterity_cells);
    BtlStatusBlank(2, g_btl_status_agility_cells);
    BtlStatusBlank(2, g_btl_status_luck_cells);
    BtlStatusBlank(3, g_btl_status_mag_atk_cells);
    BtlStatusBlank(3, g_btl_status_mag_def_cells);
    BtlStatusBlank(3, g_btl_status_rank_cells);
    BtlStatusBlank(3, g_btl_status_kind_cells);

    end = STATUS_END;
    n = (STATUS_SPELL_ROWS - 1) * STATUS_SPELL_ROW;
    g_btl_status_resist_cells.c3 = STATUS_END;
    g_btl_status_level_cells.c2 = STATUS_END;
    g_btl_status_hp_cells.c3 = STATUS_END;
    g_btl_status_hp_max_cells.c3 = STATUS_END;
    g_btl_status_sp_cells.c3 = STATUS_END;
    g_btl_status_sp_max_cells.c3 = STATUS_END;
    g_btl_status_strength_cells.c2 = STATUS_END;
    g_btl_status_vitality_cells.c2 = STATUS_END;
    g_btl_status_dexterity_cells.c2 = STATUS_END;
    g_btl_status_agility_cells.c2 = STATUS_END;
    g_btl_status_luck_cells.c2 = STATUS_END;
    g_btl_status_mag_atk_cells.c3 = STATUS_END;
    g_btl_status_mag_def_cells.c3 = STATUS_END;
    g_btl_status_rank_cells.c3 = STATUS_END;
    g_btl_status_kind_cells.c3 = STATUS_END;
    do {
        g_btl_status_spell_cells[n] = end;
        n -= STATUS_SPELL_ROW;
    } while (n >= 0);
}
