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

/* The formation preset and the seven words that follow it. */
#define BTL_PRESET_BYTES 0xC0
#define BTL_PRESET_TAIL  0xC8

/* Where the preset lands: the formation itself sits partway into the block. */
#define BTL_FORM_BASE ((u_char *)0x8004E0D0)

/* g_options fields the battle takes a copy of. */
#define OPTION_CONFIRM    1
#define OPTION_MSG_SPEED  2
#define OPTION_WINDOW_ANIM 0x23

/* The per-member byte the options block carries, one each. */
#define OPTION_MEMBER 0x1E

extern const u_char g_btl_test_party_keys[][5];
extern const u_char g_btl_test_party_keys2[][5];
extern const u_char g_btl_test_party_personas[][5];
extern const u_char g_btl_test_party_personas2[][5];
extern const u_char g_btl_test_party_names[][10];
extern u_char   g_btl_test_party;
extern u_char   g_options[];
extern u_char   g_formation_preset[];
extern u_char   g_btl_confirm;
extern u_char   g_btl_msg_speed;

extern void PersonaCreate(Char *c, int persona);

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
        int   slot;
        int   key;

        g_party[0] = 0;
        g_party[1] = 1;
        g_party[2] = 2;
        g_party[3] = 3;
        g_party[4] = 4;
        slot = 1;
        do {
            if (g_btl_encounter < BTL_TEST_HIGH) {
                key = g_btl_test_party_keys[g_btl_encounter][slot];
            } else {
                key = g_btl_test_party_keys2[g_btl_encounter
                                             - BTL_TEST_HIGH][slot];
            }
            g_chars[slot].key = key;
            c = &g_chars[slot];
            if (key > 1) {
                memcpy(c->name, g_btl_test_party_names[key],
                       sizeof(c->name));
            }
            if (key == 2) {
                c->equip[0] = BTL_TEST_WEAPON;
            }
            if (g_btl_encounter < BTL_TEST_HIGH) {
                PersonaCreate(c,
                    g_btl_test_party_personas[g_btl_encounter][slot]);
            } else {
                PersonaCreate(c,
                    g_btl_test_party_personas2[g_btl_encounter
                                               - BTL_TEST_HIGH][slot]);
            }
            slot++;
        } while (slot < BTL_PARTY);
    }

    i = 0;
    a = g_btl_actors;
    do {
        n = *party;
        if (g_chars[n].key == a->c.key) {
            memcpy(a, &g_chars[n], sizeof(Char));
        } else {
            memcpy(a, &g_chars[n], sizeof(Char));
            a->c.unk5D = 0;
        }
        party++;
        a->flags = 0;
        a->unkC8 = g_options[OPTION_MEMBER + i];
        i++;
        a++;
    } while (i < BTL_PARTY);

    memcpy(BTL_FORM_BASE, g_formation_preset, BTL_PRESET_BYTES);
    g_btl_confirm = g_options[OPTION_CONFIRM];
    ((int *)(BTL_FORM_BASE + BTL_PRESET_TAIL))[0] =
        ((int *)(g_formation_preset + BTL_PRESET_TAIL))[0];
    ((int *)(BTL_FORM_BASE + BTL_PRESET_TAIL))[1] =
        ((int *)(g_formation_preset + BTL_PRESET_TAIL))[1];
    ((int *)(BTL_FORM_BASE + BTL_PRESET_TAIL))[2] =
        ((int *)(g_formation_preset + BTL_PRESET_TAIL))[2];
    ((int *)(BTL_FORM_BASE + BTL_PRESET_TAIL))[3] =
        ((int *)(g_formation_preset + BTL_PRESET_TAIL))[3];
    ((int *)(BTL_FORM_BASE + BTL_PRESET_TAIL))[4] =
        ((int *)(g_formation_preset + BTL_PRESET_TAIL))[4];
    ((int *)(BTL_FORM_BASE + BTL_PRESET_TAIL))[5] =
        ((int *)(g_formation_preset + BTL_PRESET_TAIL))[5];
    ((int *)(BTL_FORM_BASE + BTL_PRESET_TAIL))[6] =
        ((int *)(g_formation_preset + BTL_PRESET_TAIL))[6];
    g_btl_fast_anim = g_options[OPTION_WINDOW_ANIM];
    g_btl_msg_speed = g_options[OPTION_MSG_SPEED];

    i = 0;
    if (g_btl_encounter == BTL_WEAK_ENCOUNTER) {
        a = g_btl_actors;
        n = 0;
        do {
            if (g_btl_actors[i].c.key > BTL_WEAK_KEY0 - 1
                && (g_btl_actors[i].c.key < BTL_WEAK_KEY1
                    || g_btl_actors[i].c.key == BTL_WEAK_KEY2)) {
                n = g_btl_actors[i].c.hp_max;
                if (n < 0) {
                    n += 3;
                }
                if (n >> 2 < a->c.hp) {
                    a->c.hp = n >> 2;
                }
            }
            a++;
            i++;
        } while (i < BTL_PARTY);
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/takeparty", BtlTakeParty);
#endif

