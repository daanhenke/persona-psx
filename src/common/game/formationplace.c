/* Persona 1 (JP) - moving the formation markers onto their cells.
 *   DNG 0x80090C14   ADV 0x8008CAB8   S2D 0x80081128
 *
 * DNG's copy is eight bytes longer than the other two out of the same source:
 * that overlay's object was built against an int-taking SlotInitTagged, so it
 * hands the slot over unmasked and gcc keeps the marker table's base in a saved
 * register across the loop rather than folding it into each access. Its object
 * says so with SLOT_TAGGED_INT; see slot.h.
 *
 * The preset fit test that follows it in the image is a unit of its own, in
 * formationfits.c. See formation.h.
 */
#include <persona/common/formation.h>
#include <persona/common/char.h>

/* Moves each member's marker sprite onto its cell. A member who is not on the
   grid still gets a sprite - placed from cell 0xFF, so off the right-hand side
   - and is hidden instead. */
void FormationPlaceMarkers(void)
{
    u_char *cells;
    u_char  member;
    u_char  cell;
    u_char  col;
    u_char  row;

    cells = g_formation_cell;
    for (member = 0; member < PARTY_MAX; member++) {
        g_slot_cur = &g_marker_slot[member];
        cell = cells[member];
        row = cell / GRID_W;
        col = cell % GRID_W;
        SlotInitTagged(g_formation_marker_def[member], MARKER_SLOT + member,
                       MARKER_Z, col * MARKER_XPITCH + MARKER_X0,
                       row * MARKER_YPITCH + MARKER_Y0);
        if (cells[member] == CELL_EMPTY) {
            g_slot_cur->attr |= SLOT_ATTR_HIDE;
        }
    }
}
