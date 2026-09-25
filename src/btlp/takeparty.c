/* Persona 1 (JP) - copying the party out of the field into the battle.
 *   0x8008635C BtlTakeParty  (BTLP only)
 *
 * Run once from ovl_btlp_entry, just before BtlLoadPersonas.
 *
 * Each of the five party slots names a Char record, which is copied whole into
 * the matching combatant. A member whose key has changed since the last battle
 * also has Char.unk5D cleared, so whatever the previous fight left there does
 * not carry over.
 *
 * Then the formation preset comes across, along with the three settings the
 * battle keeps its own copies of, and the per-member byte at +0xC8.
 *
 * Encounter 0xD is the exception: every member whose key is 4, 5 or 9 has its
 * HP held down to a quarter of what the field had, so that battle starts them
 * weakened.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/common/char.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/formation.h>
#include <persona/common/formation.h>

#define FORM_PRESETS 8
#define FORM_LIVE    8

/* The save block's copies, reached by address as storeparty.c writes them. */
#define g_save_actor_flag ((u_char *)0x801F2AE6)
#define g_save_fast_anim  (*(u_char *)0x801F2AEB)
#define g_save_confirm    (*(u_char *)0x801F2AC9)
#define g_save_msg_speed  (*(u_char *)0x801F2ACA)

#define BTL_PARTY 5

/* Encounters past this index the second pair of test tables. */
#define BTL_TEST_HIGH 0x11
#define BTL_TEST_WEAPON 0xEB

/* Reached by address, the way the rest of the save area is. */
#define g_party ((u_char *)0x801F256C)

/* Which keys the weakening encounter applies to. */
#define BTL_WEAK_ENCOUNTER 0xD
#define BTL_WEAK_KEY0      4
#define BTL_WEAK_KEY1      6
#define BTL_WEAK_KEY2      9

extern const u_char g_btl_test_party_keys[][5];
extern const u_char g_btl_test_party_keys2[][5];
extern const u_char g_btl_test_party_personas[][5];
extern const u_char g_btl_test_party_personas2[][5];
extern u_char   g_btl_test_party;

extern void PersonaCreate(Char *c, int persona);

/* 91.36%. The party is walked as party[i] against &g_btl_actors[i], the
   quarter is a plain / 4, and g_chars/g_party are ignore:True so splat prints
   the literals the source uses. What is left is scheduling:
   - the image loads the three save bytes and stores g_btl_confirm before the
     live row's copy. sched1 gives that copy no dependences at all here (its
     `movstrsi` has a symbol destination and conflicts with nothing), so it
     is the last insn picked and lands first. In the image something tied it
     to the settings. A copy through a varying address would do it, but gcc
     folds every pointer form back into the symbol;
   - in the test block the image sets PersonaCreate's first argument before
     the table pick and steps the Char walker in the call's delay slot. */
#ifdef NON_MATCHING
void BtlTakeParty(void)
{
    BtlActor *a;
    u_char   *party;
    int       i;
    int       n;

    party = g_party;
    /* Never entered: nothing writes g_btl_test_party and it is zero on disc.
       gcc emitted the block anyway, so it has to be here. */
    if (g_btl_test_party != 0 && g_btl_place_party != 0
        && g_btl_encounter != 5 && g_btl_encounter != 8) {
        Char *c;
        int   key;

        g_party[0] = 0;
        g_party[1] = 1;
        g_party[2] = 2;
        g_party[3] = 3;
        g_party[4] = 4;
        i = 1;
        do {
            if (g_btl_encounter < BTL_TEST_HIGH) {
                key = g_btl_test_party_keys[g_btl_encounter][i];
            } else {
                key = g_btl_test_party_keys2[g_btl_encounter
                                             - BTL_TEST_HIGH][i];
            }
            g_chars[i].key = key;
            c = &g_chars[i];
            if (key > 1) {
                memcpy(c->name, g_btl_test_party_names[key],
                       sizeof(c->name));
            }
            if (key == 2) {
                c->equip[0] = BTL_TEST_WEAPON;
            }
            if (g_btl_encounter < BTL_TEST_HIGH) {
                PersonaCreate(c,
                    g_btl_test_party_personas[g_btl_encounter][i]);
            } else {
                PersonaCreate(c,
                    g_btl_test_party_personas2[g_btl_encounter
                                               - BTL_TEST_HIGH][i]);
            }
            i++;
        } while (i < BTL_PARTY);
    }

    i = 0;
    do {
        a = &g_btl_actors[i];
        n = party[i];
        if (g_chars[n].key == a->c.key) {
            memcpy(a, &g_chars[n], sizeof(Char));
        } else {
            memcpy(a, &g_chars[n], sizeof(Char));
            a->c.unk5D = 0;
        }
        a->flags = 0;
        a->tactic = g_save_actor_flag[i];
        i++;
    } while (i < BTL_PARTY);

    memcpy(g_btl_formation_preset, g_formation_preset,
           GRID_CELLS * FORM_PRESETS);
    g_btl_confirm = g_save_confirm;
    memcpy(g_btl_formation, &g_formation_preset[GRID_CELLS * FORM_LIVE],
           GRID_CELLS);
    g_btl_fast_anim = g_save_fast_anim;
    g_btl_msg_speed = g_save_msg_speed;

    i = 0;
    if (g_btl_encounter == BTL_WEAK_ENCOUNTER) {
        do {
            n = g_btl_actors[i].c.key;
            if (n > BTL_WEAK_KEY0 - 1
                && (n < BTL_WEAK_KEY1 || n == BTL_WEAK_KEY2)) {
                if (g_btl_actors[i].c.hp_max / 4 < g_btl_actors[i].c.hp) {
                    g_btl_actors[i].c.hp = g_btl_actors[i].c.hp_max / 4;
                }
            }
            i++;
        } while (i < BTL_PARTY);
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/takeparty", BtlTakeParty);
#endif

