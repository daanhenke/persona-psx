#ifndef PERSONA_COMMON_MENUCTX_H
#define PERSONA_COMMON_MENUCTX_H

/* Persona 1 (JP) - the menu screen's context block.
 *
 * g_menu points at one of these. Its cursors live in the block itself; the
 * fields below are the ones a decompiled routine has pinned down, and the gaps
 * between them are not yet accounted for.
 */
#include <decomp/types.h>
#include <persona/common/menulist.h>

typedef struct {
    /* 0x000 */ int      member;   /* which party member the screen is on */
    /* 0x004 */ u_char   pad004[0x1BC];
    /* 0x1C0 */ int      slot_base;
    /* 0x1C4 */ u_char   pad1C4[0xC];
    /* 0x1D0 */ int      row;
    /* 0x1D4 */ u_char   pad1D4[0x1CC];
    /* 0x3A0 */ MenuList grid[2];  /* the formation grid's row, then column  */
    /* 0x3C0 */ MenuList list[4];
} MenuCtx;

extern MenuCtx *g_menu;

/* Which step of a menu is running, and whether a held button still counts.
   menu.h declares them too. */
extern short  g_menu_subsel;
extern u_char g_menu_allow_hold;

#endif
