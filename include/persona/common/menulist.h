#ifndef PERSONA_COMMON_MENULIST_H
#define PERSONA_COMMON_MENULIST_H

/* Persona 1 (JP) - cursor state for a scrolling menu list.
 *
 * A menu is an array of these, walked in 0x10 steps: ADV sets up eleven in a
 * row from 0x80068CF0 as MenuListInit(g_menu + 0x10*n, ...). MenuStepCursor
 * moves one of them from the d-pad once per frame.
 */
#include <types.h>

typedef struct {
    /* 0x00 */ int     cur;     /* current index, clamped to lo..hi         */
    /* 0x04 */ int     lo;
    /* 0x08 */ int     hi;
    /* 0x0C */ u_short delay;   /* frames left before the next auto-repeat  */
    /* 0x0E */ u_short flags;   /* MENU_* below                             */
} MenuList;                     /* 0x10 bytes */

/* Bit 0 is state, not configuration: MenuListInit sets it and MenuStepCursor
   toggles it off on the first repeat, which is how one hold gets a long delay
   (0x20 frames) before the first move and a short one (2 frames) after. */
#define MENU_FIRST_REPEAT 0x0001
#define MENU_WRAP         0x0002  /* run off one end and reappear at the other */
#define MENU_DOWN_IS_NEXT 0x0004  /* Down increments, Up decrements  */
#define MENU_RIGHT_IS_NEXT 0x0008 /* Right increments, Left decrements */
#define MENU_UP_IS_NEXT   0x0040  /* the inverse of MENU_DOWN_IS_NEXT  */
#define MENU_LEFT_IS_NEXT 0x0080  /* the inverse of MENU_RIGHT_IS_NEXT */
#define MENU_CLICK_A      0x0010  /* play sequence 1 on every step */
#define MENU_CLICK_B      0x0020  /* play sequence 3 on every step */

extern void MenuListInit(MenuList *m, int cur, int lo, int hi, u_short flags);
extern int  MenuStepCursor(MenuList *m);

#endif
