/* Persona 1 (JP) - the facility screens' helpers.  ADV only.
 *
 * The stretch between the fusion screen and the shop counters: what the
 * velvet room, the shops and the other facilities lean on - lookups in the
 * Persona and fusion tables, counts, the money, and the screens' own steps.
 * Most of it is still the original's code.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/persona.h>

#define g_persona_slots ((u_char *)0x801F2574)
#define g_money         (*(u_int *)0x801F2674)
#define g_coins         (*(u_int *)0x801F2678)

#define SLOT_EMPTY    0xFF
#define FUSE_SPECIALS 40
#define COINS_MAX     99999999

extern u_char D_800BA0E4[];     /* the fusion tables */
extern int  g_pad_pressed[];
extern void RunFrame(void);
extern int  MsgStep(void);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A1990);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A1C24);

#ifdef NON_MATCHING
/* Whether a Persona's definition lists `spell` among its first six. */
u_char PersonaDefHasSpell(short persona, short spell)
{
    int i;

    for (i = 0; i < 6; i++) {
        if (g_persona_defs[persona].raw[i] == spell) {
            return 1;
        }
    }
    return 0;
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", PersonaDefHasSpell);
#endif

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A1E7C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2140);

#ifdef NON_MATCHING
/* Whether a Persona is one of the forty the fusion tables treat specially. */
u_char FuseSpecialHas(short persona)
{
    int i;

    for (i = 0; i < FUSE_SPECIALS; i++) {
        if (D_800BA0E4[0x208 + i] == persona) {
            return 1;
        }
    }
    return 0;
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", FuseSpecialHas);
#endif

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A26DC);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A275C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2904);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2994);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2A48);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2CD0);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2E9C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2FF8);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A3388);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A3984);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A3D0C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A43BC);

#ifdef NON_MATCHING
/* How many of the sixteen Persona slots are filled. */
u_char PersonaSlotsCount(void)
{
    u_char *slots;
    u_char  i;
    u_char  n;

    slots = g_persona_slots;
    i = 0;
    n = 0;
    for (; i < 16; i++) {
        if (slots[i] != SLOT_EMPTY) {
            n++;
        }
    }
    return n;
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", PersonaSlotsCount);
#endif

#ifdef NON_MATCHING
/* Whether a Persona has reached its eighth spell slot. */
u_char PersonaSpellsFull(short n)
{
    return g_personas[n].slots >= 8;
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", PersonaSpellsFull);
#endif

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A4778);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A47F8);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A4BD0);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A4C98);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A4DA0);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A4E2C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A4E7C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A52F0);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A5560);

#ifdef NON_MATCHING
/* Keeps the message running until any button is pressed. */
void MsgWaitPress(void)
{
    goto check;
loop:
    MsgStep();
check:
    RunFrame();
    if (g_pad_pressed[0] == 0) goto loop;
    RunFrame();
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", MsgWaitPress);
#endif

#ifdef NON_MATCHING
/* How many coins the money buys, at a hundred a coin, up to what the coin
   count can still hold. */
u_int CoinsAffordable(void)
{
    u_int room;
    u_int n;

    n = g_money / 100;
    room = COINS_MAX - g_coins;
    if (room < n) {
        return room;
    }
    return n;
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", CoinsAffordable);
#endif

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A5B7C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A5BCC);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6308);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6674);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6728);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6788);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6E50);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6F2C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6FFC);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A7118);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A7578);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A76F4);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A79CC);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A7BD0);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A7D7C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8104);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A81C8);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8200);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8448);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8928);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8B88);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A91C8);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A95B8);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A97A4);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A9868);
