/* Persona 1 (JP) - a character joining the party.  ADV only.
 *   0x800AFAD0 CharJoin    0x800AFE8C CharGrow
 *
 * Script command 31 fills record `n` for the character `key` out of their
 * starting template (DNG's CharInit does the same for a new game): full hp
 * and sp, the equipment, the name and the stats. Anyone but the hero is then
 * grown to the hero's level, so a latecomer does not join at level five.
 * The Persona list starts empty.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/char.h>

#define HERO      1
#define STAT_CAP  999

/* The name is copied as eight bytes, the two words the template's name
   starts with; DNG's CharInit copies all ten. */
typedef struct {
    u_char b[8];
} Name8;

/* A byte table reached by a literal address that is plainly wrong: it
   stores into low memory, indexed by the record. */
#define g_join_flag ((u_char *)0x22)

/* Growth rows, one entry to every two levels: five stat rows of 50 a
   character, and a hit point row from the third character (the hero and the
   second key have none) - btlp/levelup.c reads the same two tables. */
extern u_char g_char_stat_growth[];
extern u_char g_char_hp_growth[];

#define GROWTH_COLS 50

void CharGrow(u_char from, u_char to, u_char n, u_char key);
extern void CharSetLevelExp(u_char level, u_char slot);
extern void CharApplyStats(u_char chr);
extern void CharRecalcStats(u_char chr);

void CharJoin(u_char n, u_char key, u_char level)
{
    CharTemplate *t;
    Char         *c;

    t = &g_char_templates[key - 1];
    *(Name8 *)g_chars[n].name = *(Name8 *)t->name;
    g_chars[n].hp_max = t->hp;
    g_chars[n].sp_max = t->sp;
    g_chars[n].unk10 = 0;
    g_chars[n].unk14 = 0;
    g_chars[n].unk18 = 0;
    g_chars[n].unk1C = 0;
    g_chars[n].equip[0] = t->equip[0];
    g_chars[n].equip[1] = t->equip[1];
    g_chars[n].equip[2] = t->equip[2];
    g_chars[n].equip[3] = t->equip[3];
    g_chars[n].equip[4] = t->equip[4];
    g_chars[n].equip[5] = t->equip[5];
    g_chars[n].equip[6] = t->equip[6];
    g_chars[n].key = key;
    g_chars[n].status = 0;
    g_chars[n].ail_level = 0;
    g_chars[n].level = level;
    g_chars[n].stat_base[0] = t->stat[0];
    g_chars[n].stat_base[1] = t->stat[1];
    g_chars[n].stat_base[2] = t->stat[2];
    g_chars[n].stat_base[3] = t->stat[3];
    g_chars[n].stat_base[4] = t->stat[4];
    g_chars[n].mag_atk = 1;
    g_chars[n].mag_def = 1;
    c = &g_chars[n];
    if (key != HERO) {
        CharGrow(1, g_chars[0].level, n, key);
        CharSetLevelExp(g_chars[0].level, n);
    }
    if (g_chars[n].hp_max > STAT_CAP) {
        g_chars[n].hp_max = STAT_CAP;
    }
    if (g_chars[n].sp_max > STAT_CAP) {
        g_chars[n].sp_max = STAT_CAP;
    }
    c->hp = g_chars[n].hp_max;
    g_chars[n].sp = g_chars[n].sp_max;
    g_chars[n].stat[0] = g_chars[n].stat_base[0];
    g_chars[n].stat[1] = g_chars[n].stat_base[1];
    g_chars[n].stat[2] = g_chars[n].stat_base[2];
    g_chars[n].stat[3] = g_chars[n].stat_base[3];
    g_chars[n].stat[4] = g_chars[n].stat_base[4];
    g_chars[n].unk56 = key == HERO ? 5 : g_chars[0].unk56;
    c->unk1C = ExpToLevel(c->unk56, n, 0);
    c->entry = CHAR_NO_ENTRY;
    c->list[0] = CHAR_NO_ENTRY;
    c->list[1] = CHAR_NO_ENTRY;
    c->list[2] = CHAR_NO_ENTRY;
    c->pad5B[0] = 0;
    c->resist = t->resist;
    c->unk5D = 0;
    g_join_flag[n] = 0;
    c->blocked = 0;
    CharApplyStats(n);
    CharRecalcStats(n);
}

/* Levels record `n` up from `from` to `to` the way the battle does: each
   level adds the character's growth rows to the hit points and the five base
   stats, and four plus a share of the magic pair to the SP. */
/* 92.91%: the original moves g_char_stat_growth's address out of the loop
   and this keeps it in: loop.c sees its set with a lifetime of one insn and
   a saving of one and finds it not worth a register (cc1 -dL). Every row
   form that keeps the one address computation - index, pointer arithmetic,
   base first, a two-dimensional view - and every loop shape gives the same
   lifetime; a local for the base moves it but frees the budget for mag_def,
   which the original keeps in the loop. */
#ifdef NON_MATCHING
void CharGrow(u_char from, u_char to, u_char n, u_char key)
{
    Char   *c;
    u_char *row;
    int     i;

    c = &g_chars[n];
    for (i = from - 1; i <= to - 1; i++) {
        c->hp_max += g_char_hp_growth[(key - 2) * GROWTH_COLS + i / 2];
        c->sp_max += (c->mag_atk * 100 / 125 + c->mag_def) / 49 + 4;
        row = &g_char_stat_growth[key * CHAR_STATS * GROWTH_COLS + i / 2];
        c->stat_base[0] += row[0 * GROWTH_COLS];
        c->stat_base[1] += row[1 * GROWTH_COLS];
        c->stat_base[2] += row[2 * GROWTH_COLS];
        c->stat_base[3] += row[3 * GROWTH_COLS];
        c->stat_base[4] += row[4 * GROWTH_COLS];
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/game/charjoin", CharGrow);
#endif
