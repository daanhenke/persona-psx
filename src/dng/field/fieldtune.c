/* Persona 1 (JP) - the zone tunes, and a character's starting record.
 * DNG only.
 *   0x80070DAC FieldZoneTunes
 *   0x80070FB0 CharInit
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libsnd.h>
#include <memory.h>
#include <persona/common/char.h>
#include <persona/dng/field.h>

#define HERE (g_floor_grid[g_dng->pos[POS_Y]][g_dng->pos[POS_X]])

/* The pair of handles the playing tunes use, and the sequences each pair
   comes from. */
#define TUNE_BGM  15
#define TUNE_IDLE 16

#define SWAP_TUNES(bgm, idle)                                                  \
    {                                                                          \
        SsSetNck(g_seq_handles[TUNE_BGM]);                                     \
        SsSetNck(g_seq_handles[TUNE_IDLE]);                                    \
        g_idle_seq = g_seq_handles[TUNE_IDLE] =                                \
            SsSeqOpen((u_long *)(SEQ_BASE + SEQ_OFFSETS[idle]), g_vab_handles[1]); \
        g_bgm_seq = g_seq_handles[TUNE_BGM] =                                  \
            SsSeqOpen((u_long *)(SEQ_BASE + SEQ_OFFSETS[bgm]), g_vab_handles[1]); \
        if (!quiet) {                                                          \
            SsSeqPlay(g_bgm_seq, 1, 0);                                        \
        }                                                                      \
    }

void FieldZoneTunes(int quiet)
{
    if (g_tile_defs[HERE].flags & TILE_ZONE_TUNE) {
        if (!(g_bgm_flags & BGM_ZONE)) {
            SWAP_TUNES(15, 16);
            g_bgm_flags |= BGM_ZONE;
        }
    } else if (g_bgm_flags & BGM_ZONE) {
        SWAP_TUNES(12, 13);
        g_bgm_flags &= ~BGM_ZONE;
    }
}

/* Fills record `rec` from starting record `tmpl`: full hp and sp, the
   equipment, name, stats and resistance, level 5, and the derived values
   10 to 17 until the equipment is weighed. */
void CharInit(int rec, int tmpl)
{
    Char *c;

    c = &g_chars[rec];
    c->hp = g_char_templates[tmpl].hp;
    c->hp_max = g_char_templates[tmpl].hp;
    c->sp = g_char_templates[tmpl].sp;
    c->sp_max = g_char_templates[tmpl].sp;
    c->equip[0] = g_char_templates[tmpl].equip[0];
    c->equip[1] = g_char_templates[tmpl].equip[1];
    c->equip[2] = g_char_templates[tmpl].equip[2];
    c->equip[3] = g_char_templates[tmpl].equip[3];
    c->equip[4] = g_char_templates[tmpl].equip[4];
    c->equip[5] = g_char_templates[tmpl].equip[5];
    c->equip[6] = g_char_templates[tmpl].equip[6];
    c->stat[0] = g_char_templates[tmpl].stat[0];
    c->stat_base[0] = g_char_templates[tmpl].stat[0];
    c->stat[1] = g_char_templates[tmpl].stat[1];
    c->stat_base[1] = g_char_templates[tmpl].stat[1];
    c->stat[2] = g_char_templates[tmpl].stat[2];
    c->stat_base[2] = g_char_templates[tmpl].stat[2];
    c->stat[3] = g_char_templates[tmpl].stat[3];
    c->stat_base[3] = g_char_templates[tmpl].stat[3];
    c->stat[4] = g_char_templates[tmpl].stat[4];
    c->stat_base[4] = g_char_templates[tmpl].stat[4];
    c->resist = g_char_templates[tmpl].resist;
    c->melee_atk = 10;
    c->melee_hit = 11;
    c->gun_atk = 12;
    c->gun_hit = 13;
    c->defence = 14;
    c->evade = 15;
    c->mag_atk = 16;
    c->mag_def = 17;
    c->key = tmpl + 1;
    c->level = 5;
    c->unk56 = 5;
    bcopy(g_char_templates[tmpl].name, c->name, 10);
}
