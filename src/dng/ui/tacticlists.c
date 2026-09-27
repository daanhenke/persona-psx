/* Persona 1 (JP) - the main menu's tactic lists, and a slot highlighter.
 * DNG only.
 *   0x80090078 TacticListStep  0x80090160 SlotsHighlight
 *   0x80090218 MenuStepNone
 *
 * The main menu keeps one MenuList per party member for their battle tactic
 * (g_tactic_lists, opened from g_party_tactic) and a cursor over the members
 * (g_tactic_member). The step moves the chosen member's list and writes all
 * five back.
 */
#include <decomp/types.h>
#include <persona/common/menulist.h>
#include <persona/common/slot.h>

#define g_cfg          ((u_char *)0x801F2AC4)
#define g_slots        ((Slot *)0x800DC10C)

extern MenuList g_tactic_member;
extern MenuList g_tactic_list0, g_tactic_list1, g_tactic_list2, g_tactic_list3,
                g_tactic_list4;

void TacticListStep(void)
{
    MenuList *m;
    u_char   *cfg = g_cfg;

    switch (g_tactic_member.cur) {
    case 0:
        return;
    case 1:
        m = &g_tactic_list0;
        break;
    case 2:
        m = &g_tactic_list1;
        break;
    case 3:
        m = &g_tactic_list2;
        break;
    case 4:
        m = &g_tactic_list3;
        break;
    case 5:
        m = &g_tactic_list4;
        break;
    }
    if (MenuStepCursor(m)) {
        /* g_party_tactic, through the config block's base. */
        cfg[0x22] = g_tactic_list0.cur;
        cfg[0x23] = g_tactic_list1.cur;
        cfg[0x24] = g_tactic_list2.cur;
        cfg[0x25] = g_tactic_list3.cur;
        cfg[0x26] = g_tactic_list4.cur;
    }
}

/* Slots `first` to `last` dimmed, bar `sel`, which is lit and nudged two
   pixels right. */
void SlotsHighlight(u_char first, u_char last, u_char sel)
{
    Slot  *s;
    u_char i;

    for (i = first; i <= last; i++) {
        s = &g_slots[i];
        if (i == sel) {
            SlotSetBrightness(i, 0x80);
            s->unk18 = 2;
        } else {
            SlotSetBrightness(i, 0x40);
            s->unk18 = 0;
        }
    }
}

void MenuStepNone(void)
{
}
