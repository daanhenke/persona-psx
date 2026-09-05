#ifndef PERSONA_COMMON_MENU_H
#define PERSONA_COMMON_MENU_H

/* Persona 1 (JP) - menu input poll.
 *
 * The routine is compiled into DNG, S2D and ADV rather than called across the
 * boundary. S2D and ADV share src/p1-jp/common/menupoll.c; DNG's copy is two
 * instructions shorter and keeps src/p1-jp/dng/menupoll.c. Everything below is
 * identical between the two sources, which is why it lives here.
 */
#include <types.h>

extern u_short *g_menu;   /* -> object whose first u16 feeds the store */
extern u_char   g_menu_blink;
extern u_char   g_menu_allow_hold;
extern short    g_menu_subsel;
extern short    g_menu_sel;

extern int  MenuStepCursor(u_short *m);   /* really a MenuList *, see menulist.c */
extern void UpdateMenuSprites(int arg);
extern void DrawStatusHud(void);

/* InputCheckAcceptA and InputCheckAcceptB are deliberately absent. S2D and ADV
   were built against prototypes returning u_char and DNG against ones returning
   int; the two instructions of result masking that difference produces are the
   only thing separating the two sources, so each declares its own pair. Giving
   them one shared prototype here would cost one of the two matches. */

extern void MenuPollInput(void);

#endif
