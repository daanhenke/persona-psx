/* Persona 1 (JP) - the formation screen's member sprites.
 *
 *   ADV 0x8008D064 ..
 *
 * The second of the two formation units: one sprite per party member on the
 * grid, the row-compaction that runs after a preset is loaded, and the clear.
 * The grid itself is in formation.c. See formation.h.
 */
#include <persona/common/formation.h>
#include <persona/common/char.h>

/* The character's key doubles as its portrait number, counting from one.

   The member counter is an int: the party bound is compared signed, which is
   what puts the always-false `g_party_last < 0` guard ahead of the loop. */
void FormationDrawMembers(void)
{
    Char *chars;
    int   member;

    chars = g_chars;
    for (member = 0; member <= g_party_last; member++) {
        FormationSetMemberSprite(member, chars[g_party_at[member]].key - 1);
    }
}

/* Placing one member's sprite does not come out of the C yet; the overlays
   take it from asm, so it is kept here for the progress build only. */
#ifdef NON_MATCHING
/* Places one member's marker on its cell, or clears the slot when the member
   is off the grid. */
void FormationSetMemberSprite(u_char member, u_char portrait)
{
    u_char cell;
    u_char col;
    u_char row;
    int    dx;

    cell = g_formation_cell[member];
    if (cell != CELL_EMPTY) {
        row = cell / GRID_W;
        col = cell % GRID_W;
        g_slot_cur = &g_member_slot[member];
        dx = row * MEMBER_ROW_X - MEMBER_X0;
        SlotInitTagged(&g_formation_member_def, MEMBER_SLOT + member,
                       MEMBER_Z0 - (row * GRID_W + col),
                       col * MEMBER_COL_X - dx,
                       row * MEMBER_ROW_Y + col * MEMBER_COL_Y + MEMBER_Y0);
        g_slot_cur->u_add = portrait * MEMBER_PORTRAIT_W;
    } else {
        SlotClear(MEMBER_SLOT + member);
    }
}
#endif
