/* Persona 1 (JP) - the levels a won fight's experience buys.  BTLP only.
 *   0x80097A50 BtlLevelUpParty
 *
 * Runs on the way out of a won fight, after BtlBattleResults has handed the
 * experience out and raised g_btl_level_up for anyone it carried over a level.
 * Outside the scripted fights the results board is put up first and held for
 * a key.
 *
 * The level-up fanfare - the sound pack's entry just before the voices - is
 * read into the voice buffer and opened on the BGM slot, and the edit board is
 * put up. Each member whose experience covers the next level takes levels one
 * at a time while it does, up to 99: hit points from the member's growth row,
 * or four and a fourteenth of vitality and luck for the protagonist, who also
 * takes three points to spend; spell points from the two numbers the
 * negotiation weighs; and for everyone but the protagonist a column of each
 * stat's growth row.
 *
 * The board then counts the gains up. The protagonist spends the points on the
 * five stats with the cursor - confirm or right adds one, cancel or left takes
 * one back, the abort key starts again - and is asked whether that is the
 * build; everyone else has the gains counted in a step every three frames.
 * Whatever changed is drawn in the lit palette.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/choice.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/message.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/text.h>
#include <persona/common/char.h>
#include <persona/main/cd.h>

/* The fanfare's entry in the sound pack: the one before the voices. */
#define LEVEL_FANFARE (BTL_VOICE_FIRST - 1)

/* The highest level, stat and maximum, and what the five stats come to when
   every one is as high as it goes. */
#define LEVEL_MAX        99
#define LEVEL_STAT_MAX   99
#define LEVEL_MAXIMA     999
#define LEVEL_STATS_FULL (LEVEL_STAT_MAX * CHAR_STATS)

/* The protagonist, the points a level gives him to spend, and what his hit
   points grow by. */
#define LEVEL_HERO        1
#define LEVEL_POINTS      3
#define LEVEL_HERO_HP     4
#define LEVEL_HERO_HP_DIV 14

/* Everyone else grows by the tables: a row of fifty columns per key from key
   2 on, five rows a key for the stats, and a level reads its column halved. */
#define LEVEL_GROWN_FIRST 2
#define LEVEL_GROWTH_COLS 50

/* The spell points a level adds: the two numbers, the first worth a quarter
   less, over forty-nine, and four. */
#define LEVEL_SP_WEIGHT 1.25
#define LEVEL_SP_DIV    49.0
#define LEVEL_SP_BASE   4.0

/* The edit board's numbers: the eight derived values from melee_atk on, then
   the five stats, and the two palettes they are drawn in. */
#define LEVEL_DERIVED     8
#define LEVEL_NUMBERS     (LEVEL_DERIVED + CHAR_STATS)
#define LEVEL_CLUT_SAME   0x20
#define LEVEL_CLUT_CHANGE 0x21

/* Set on the board's last record while nothing is being chosen: the cursor. */
#define LEVEL_CURSOR_OFF BTL_OBJ_HIDDEN

/* How long the board is held before the counting starts, and where the line
   of points left goes. */
#define LEVEL_HOLD_FRAMES 60
#define LEVEL_MSG_X       0x10
#define LEVEL_MSG_Y       0x10

/* The fanfare's tick, and the cursor's. */
#define LEVEL_SE_TICK 0

/* The stat the cursor is on, the eight derived values before the gains, and
   the cursor's neighbour table - up, down - and spots. */
extern short             g_btl_level_row;
extern u_short           g_btl_level_before[LEVEL_DERIVED];
extern const u_char      g_btl_level_nav[CHAR_STATS][2];
extern BtlMenuSpot       g_btl_level_spots[CHAR_STATS];
extern const u_char      g_btl_msg_points_left[];

void BtlLevelUpParty(void)
{
    int          i;
    int          hero_clut;
    int          party_clut;
    int          confirmed;
    int          hp_gain;
    int          sp_gain;
    int         *exp;
    BtlObj      *board;
    signed char *kept;
    signed char  gained[CHAR_STATS];
    CdlLOC       loc;
    BtlSoundBank bank;
    BtlActor    *a;
    u_char      *stat;
    u_short     *derived;
    u_char      *rows;
    u_char      *growth;
    signed char *gain;
    BtlGfxCell  *cell;
    BtlGfxText  *text;
    int          row;
    double       sp_first;
    double       sp_second;
    int          points;
    int          kept_points;
    int          sum;
    int          done;
    int          choice;
    int          keys;
    int          mask;
    int          j;

    if (g_btl_scripted == 0) {
        BtlPickSettle();
        BtlRetractMarkers();
        BtlOpenBoard1F();
        do {
            BtlDrawFrame();
        } while (g_btl_pad1_edge == 0);
        BtlCloseBoard1F();
        BtlDrawFrame();
    }
    if (g_btl_level_up == 0) {
        return;
    }

    CdIntToPos(g_btl_pack_sectors[LEVEL_FANFARE] + g_btl_voice_base, &loc);
    CdReadFileToAddrAsync((CdlFILE *)&loc,
                          g_btl_pack_sectors[LEVEL_FANFARE + 1]
                              - g_btl_pack_sectors[LEVEL_FANFARE],
                          (u_long *)BTL_VOICE_BUFFER);
    while (g_cd_busy != -1) {
        BtlDrawFrame();
    }
    bank.vb = g_btl_voice_vb;
    bank.vh = g_btl_voice_vh;
    bank.seq = g_btl_voice_seq;
    bank.nsep = BTL_VOICE_SEPS;
    BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);

    board = BtlObjLast(BtlOpenEditBoard());
    g_btl_level_row = 0;
    cell = g_btl_menu_cursor;
    for (j = 0; j < BTL_CURSOR_CELLS; j++) {
        cell->x = g_btl_level_spots[g_btl_level_row].x - BTL_CURSOR_NUDGE;
        cell->y = g_btl_level_spots[g_btl_level_row].y;
        cell++;
    }

    for (i = 0, kept = gained; i < BTL_PARTY; i++) {
        a = &g_btl_actors[i];
        if (a->c.key == 0) {
            continue;
        }
        points = 0;
        if (a->c.unk18 > 0) {
            board->attr |= LEVEL_CURSOR_OFF;
            continue;
        }

        hp_gain = 0;
        sp_gain = 0;
        for (j = CHAR_STATS - 1; j >= 0; j--) {
            kept[j] = 0;
        }
        exp = &g_level_exp[a->c.level];
        stat = g_btl_actors[i].c.stat_base;
        while (a->c.level < LEVEL_MAX) {
            if ((u_int)a->c.unk14 < (u_int)*exp) {
                break;
            }
            if (a->c.key >= LEVEL_GROWN_FIRST) {
                row = a->c.key - LEVEL_GROWN_FIRST;
                growth = g_char_hp_growth + row * LEVEL_GROWTH_COLS;
                hp_gain += growth[(a->c.level - 1) / 2];
                j = 0;
                gain = kept;
                rows = g_char_stat_growth;
                do {
                    j++;
                    /* Add the row address last to preserve the original operand order. */
                    growth = (u_char *)(a->c.key * CHAR_STATS * LEVEL_GROWTH_COLS
                                        + (u_int)rows);
                    *gain += growth[(a->c.level - 1) / 2];
                    rows += LEVEL_GROWTH_COLS;
                    gain++;
                } while (j < CHAR_STATS);
            } else {
                points += LEVEL_POINTS;
                hp_gain += LEVEL_HERO_HP
                         + (a->c.stat[STAT_VITALITY] + a->c.stat[STAT_LUCK]) / LEVEL_HERO_HP_DIV;
            }
            sp_first = a->unk3A;
            sp_second = a->unk3C;
            sp_gain += (sp_first / LEVEL_SP_WEIGHT + sp_second) / LEVEL_SP_DIV + LEVEL_SP_BASE;
            a->c.unk14 -= *exp;
            a->c.level++;
            a->c.hp = a->c.hp_max;
            a->c.sp = a->c.sp_max;
            a->c.unk18 = a->c.level < LEVEL_MAX ? exp[1] - a->c.unk14 : 0;
            exp = &g_level_exp[a->c.level];
        }

        BtlApplyPersona(a);
        BtlRecalcStats(a);
        BtlFillEditBoard(a);
        for (j = LEVEL_NUMBERS - 1, text = &g_btl_edit_numbers[LEVEL_NUMBERS - 1]; j >= 0; j--) {
            text->clut = LEVEL_CLUT_SAME;
            text--;
        }
        for (j = 0; j < LEVEL_HOLD_FRAMES; j++) {
            board->attr |= LEVEL_CURSOR_OFF;
            BtlDrawFrame();
        }
        derived = &a->c.melee_atk;
        for (j = 0; j < LEVEL_DERIVED; j++) {
            g_btl_level_before[j] = derived[j];
        }

        if (a->c.key == LEVEL_HERO) {
            kept_points = points;
            sum = 0;
            for (j = 0; j < CHAR_STATS; j++) {
                kept[j] = stat[j];
                sum += stat[j];
            }
            if (sum < LEVEL_STATS_FULL) {
                BtlSetInsert(0, (const u_char *)points);
                BtlOpenMessage(0, 0, g_btl_msg_points_left, LEVEL_MSG_X, LEVEL_MSG_Y);
            }
            /* Each counting loop keeps its own highlight palette in a saved register. */
            hero_clut = LEVEL_CLUT_CHANGE;
            do {
                do {
                    done = 1;
                    keys = BtlMenuKey();
                    if (keys & (PAD_UP | PAD_DOWN)) {
                        BtlSePlay(PICK_SE_BANK, LEVEL_SE_TICK);
                    }
                    if (keys & PAD_UP) {
                        g_btl_level_row = g_btl_level_nav[g_btl_level_row][0];
                    }
                    if (keys & PAD_DOWN) {
                        g_btl_level_row = g_btl_level_nav[g_btl_level_row][1];
                    }
                    if ((keys & (g_btl_key_confirm | PAD_RIGHT)) && points > 0
                        && stat[g_btl_level_row] < LEVEL_STAT_MAX) {
                        BtlSePlay(BTL_BGM_SLOT, LEVEL_SE_TICK);
                        points--;
                        stat[g_btl_level_row]++;
                    }
                    if ((keys & (g_btl_key_cancel | PAD_LEFT))
                        && stat[g_btl_level_row] > kept[g_btl_level_row]) {
                        BtlSePlay(BTL_BGM_SLOT, LEVEL_SE_TICK);
                        points++;
                        stat[g_btl_level_row]--;
                    }
                    if (keys & g_btl_key_abort) {
                        BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                        for (j = 0; j < CHAR_STATS; j++) {
                            stat[j] = kept[j];
                        }
                        points = kept_points;
                    }
                    /* Accumulate in steps to keep the original key-load registers. */
                    mask = g_btl_key_confirm | PAD_RIGHT | PAD_LEFT;
                    mask |= g_btl_key_cancel;
                    mask |= g_btl_key_abort;
                    if ((keys & mask)
                        && sum < LEVEL_STATS_FULL) {
                        BtlSetInsert(0, (const u_char *)points);
                        BtlOpenMessage(0, 0, g_btl_msg_points_left, LEVEL_MSG_X, LEVEL_MSG_Y);
                    }
                    BtlApplyPersona(a);
                    BtlRecalcStats(a);
                    BtlFillEditBoard(a);
                    /* Form each record's address before selecting clut; indexing the
                       field directly makes gcc bias the row pointers by nine bytes. */
                    for (j = 0; j < CHAR_STATS; j++) {
                        if (kept[j] != stat[j]) {
                            (g_btl_edit_numbers + LEVEL_DERIVED + j)->clut = hero_clut;
                        } else {
                            (g_btl_edit_numbers + LEVEL_DERIVED + j)->clut = LEVEL_CLUT_SAME;
                        }
                    }
                    for (j = 0; j < LEVEL_DERIVED; j++) {
                        if (g_btl_level_before[j] != derived[j]) {
                            (g_btl_edit_numbers + j)->clut = hero_clut;
                        } else {
                            (g_btl_edit_numbers + j)->clut = LEVEL_CLUT_SAME;
                        }
                    }
                    if (hp_gain > 0) {
                        if (++a->c.hp_max >= LEVEL_MAXIMA) {
                            done = 1;
                            a->c.hp_max = LEVEL_MAXIMA;
                            hp_gain = 0;
                        } else if (--hp_gain != 0) {
                            done = 0;
                        }
                    }
                    if (sp_gain > 0) {
                        if (++a->c.sp_max >= LEVEL_MAXIMA) {
                            done = 1;
                            a->c.sp_max = LEVEL_MAXIMA;
                            sp_gain = 0;
                        } else if (--sp_gain != 0) {
                            done = 0;
                        }
                    }
                    a->c.hp = a->c.hp_max;
                    a->c.sp = a->c.sp_max;
                    for (j = 0, cell = g_btl_menu_cursor; j < BTL_CURSOR_CELLS; j++) {
                        cell->x = g_btl_level_spots[g_btl_level_row].x - BTL_CURSOR_NUDGE;
                        cell->y = g_btl_level_spots[g_btl_level_row].y;
                        cell++;
                    }
                    board->attr &= ~LEVEL_CURSOR_OFF;
                    BtlDrawFrame();
                    sum = 0;
                    for (j = 0; j < CHAR_STATS; j++) {
                        sum += stat[j];
                    }
                } while ((points != 0 && sum < LEVEL_STATS_FULL) || done == 0);

                BtlCloseMessage(0);
                BtlOpenChoice1();
                do {
                    choice = BtlChoiceUpdate(&g_btl_choice1_row);
                    if (choice != BTL_MENU_WAIT) {
                        if (choice == 0) {
                            BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                            BtlCloseChoice1();
                            confirmed = 1;
                        } else {
                            BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                            BtlCloseChoice1();
                            for (j = 0; j < CHAR_STATS; j++) {
                                stat[j] = kept[j];
                            }
                            points = kept_points;
                            if (sum < LEVEL_STATS_FULL) {
                                BtlSetInsert(0, (const u_char *)points);
                                BtlOpenMessage(0, 0, g_btl_msg_points_left, LEVEL_MSG_X,
                                               LEVEL_MSG_Y);
                            }
                            confirmed = 0;
                        }
                    }
                    BtlDrawFrame();
                } while (choice == BTL_MENU_WAIT);
            } while (confirmed == 0);
            board->attr |= LEVEL_CURSOR_OFF;
        } else {
            party_clut = LEVEL_CLUT_CHANGE;
            board->attr |= LEVEL_CURSOR_OFF;
            do {
                done = 1;
                if (hp_gain > 0) {
                    if (++a->c.hp_max >= LEVEL_MAXIMA) {
                        a->c.hp_max = LEVEL_MAXIMA;
                        hp_gain = 0;
                    } else if (--hp_gain != 0) {
                        done = 0;
                    }
                }
                if (sp_gain > 0) {
                    if (++a->c.sp_max >= LEVEL_MAXIMA) {
                        done = 1;
                        a->c.sp_max = LEVEL_MAXIMA;
                        sp_gain = 0;
                    } else if (--sp_gain != 0) {
                        done = 0;
                    }
                }
                for (j = 0; j < CHAR_STATS; j++) {
                    /* A separate stat index keeps its pointer step after the text's. */
                    row = j;
                    if (kept[j] > 0 && stat[row] < LEVEL_STAT_MAX) {
                        (g_btl_edit_numbers + LEVEL_DERIVED + j)->clut = party_clut;
                        stat[row]++;
                        stat[row] = CHAR_GROW_CLAMP(stat[row], LEVEL_STAT_MAX);
                        if (--kept[j] != 0) {
                            done = 0;
                        }
                    }
                }
                a->c.hp = a->c.hp_max;
                a->c.sp = a->c.sp_max;
                BtlApplyPersona(a);
                BtlRecalcStats(a);
                BtlFillEditBoard(a);
                for (j = 0; j < LEVEL_DERIVED; j++) {
                    if (g_btl_level_before[j] != derived[j]) {
                        (g_btl_edit_numbers + j)->clut = party_clut;
                    } else {
                        (g_btl_edit_numbers + j)->clut = LEVEL_CLUT_SAME;
                    }
                }
                BtlDrawFrame();
                BtlDrawFrame();
                BtlDrawFrame();
                BtlSePlay(BTL_BGM_SLOT, LEVEL_SE_TICK);
            } while (done == 0);
        }

        BtlFaceClose();
        if (a->c.key != LEVEL_HERO) {
            while (g_btl_pad1_edge == 0) {
                BtlDrawFrame();
            }
        }
    }
    BtlShutEditBoard();
    BtlSoundClose(BTL_BGM_SLOT);
}
