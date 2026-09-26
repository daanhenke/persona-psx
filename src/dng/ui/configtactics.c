/* Persona 1 (JP) - the config screen's tactics page.  DNG only.
 * DNG's copy of src/adv/ui/configtactics.c.
 *   0x80075BE0 ConfigStepTactics
 *
 * Up and down pick a party member, left and right that member's tactic in
 * battle, one of three. Each member's row carries a marker under the tactic
 * it has, and the member under the cursor's marker flickers. Cancelling
 * redraws the battle page and steps back to it.
 */
#define SLOT_SETPOS_INT
#define SLOT_SETPOS_SLOT_INT
#define SLOT_TAGGED_INTXY
#define TILEMAP_INT_COUNT
#define PERSONAPAGE_DNG
#include <decomp/types.h>

/* This unit's calls to SlotSetFlicker pass the slot unmasked. */
#define SLOT_FLICKER_INT

#include <persona/common/menuctx.h>
#include <persona/common/formation.h>
#include <persona/common/slot.h>

/* One byte per party member, saved with the game; the battle reads it as
   each actor's tactic. Reached by hardcoded address. */
#define g_party_tactic ((u_char *)0x801F2AE6)

#define CURSOR_SLOT  1
#define MARKER_SLOT0 2
#define SLOT_Z       0x42
#define CURSOR_X     0x48
#define ROW_Y0       0x48
#define ROW_PITCH    0x18
#define TACTIC_X0    0xA0
#define TACTIC_STEP  40
#define TACTIC_LAST  2

/* Tested unmasked, as everywhere in DNG's menus. */
extern int    InputCheckAcceptB(int repeat);
extern void ConfigRedrawBattlePage(void);

void ConfigStepTactics(void)
{
    int i;

    if (MenuStepCursor(&g_menu->member_list)) {
        SlotSetPos(CURSOR_SLOT, SLOT_Z, CURSOR_X,
                   g_menu->member_list.cur * ROW_PITCH + ROW_Y0);
        i = g_menu->member_list.cur;
        MenuListInit(&g_menu->list[1], g_party_tactic[i],
                     0, TACTIC_LAST, MENU_WRAP | MENU_RIGHT_IS_NEXT | MENU_CLICK_A);
    } else if (MenuStepCursor(&g_menu->list[1])) {
        i = g_menu->member_list.cur;
        g_party_tactic[i] = g_menu->list[1].cur;
    }

    for (i = 0; i <= g_party_last; i++) {
        int slot = i + MARKER_SLOT0;

        SlotSetPos(slot, SLOT_Z,
                   g_party_tactic[i] * TACTIC_STEP + TACTIC_X0,
                   i * ROW_PITCH + ROW_Y0);
        if (i == g_menu->member_list.cur) {
            SlotSetFlicker(slot, 1);
        } else {
            SlotSetFlicker(slot, 0);
        }
    }

    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        ConfigRedrawBattlePage();
        g_menu_subsel--;
    }
}
