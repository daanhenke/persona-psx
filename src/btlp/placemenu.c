/* Persona 1 (JP) - the row the party stands in.  BTLP only.
 *   0x8009E000 BtlPlaceMenu
 *
 * Entry 4 of g_btl_pick_command: the command that puts the party's own
 * formation up and lets a member be moved between the front and the back.
 * It runs its own frame loop rather than being ticked, the way every other
 * command in that table does.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>

INCLUDE_ASM("btlp/nonmatchings/placemenu", BtlPlaceMenu);
