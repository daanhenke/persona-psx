#ifndef PERSONA_BTLP_DEBUG_H
#define PERSONA_BTLP_DEBUG_H

#include <persona/common/char.h>

/* The debug page, which BtlOrdersMenu hands to while the cancel key is held
   with g_btl_debug_hud raised. debugmenu.c. */
extern int BtlDebugMenu(void);

/* Holds the game from one press of the page key to the next, taking the
   HUD's code from the first pad meanwhile. No caller is left. debughud.c. */
extern void BtlDebugPause(void);

/* One routine per row of that page, zero where the row is handled in place or
   does nothing. Every handler answers zero, so the round carries on as though
   nothing had been ordered. debugmenu.c. */
extern int (*g_btl_debug_actions[])(void);

/* The rows. BtlDebugLoadGfx is in the image but no longer in the table. */
extern int BtlDebugLoadGfx(void);       /* debugmenu.c   */
extern int BtlDebugSummon(void);        /* debugmenu.c   */
extern int BtlDebugMemberAilment(void); /* debugailment.c */
extern int BtlDebugEnemyAilment(void);  /* debugailment.c */
extern int BtlDebugEditMember(void);    /* debugedit.c   */
extern int BtlDebugEditFlags(void);     /* debugflags.c  */

/* Steps one equipment slot through the item table: down, up, or with
   neither all the way to the top. debugequip.c. */
extern void BtlDebugStepEquip(Char *c, int slot, int dir);

#endif
