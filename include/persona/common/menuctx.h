#ifndef PERSONA_COMMON_MENUCTX_H
#define PERSONA_COMMON_MENUCTX_H

/* Persona 1 (JP) - the menu screen's context block.
 *
 * g_menu points at one of these. Its cursors live in the block itself; the
 * fields below are the ones a decompiled routine has pinned down, and the gaps
 * between them are not yet accounted for.
 */
#include <types.h>
#include <persona/common/menulist.h>

typedef struct {
    /* 0x000 */ int      member;   /* which party member the screen is on */
    /* 0x004 */ u_char   pad004[0x1BC];
    /* 0x1C0 */ int      slot_base;
    /* 0x1C4 */ u_char   pad1C4[0xC];
    /* 0x1D0 */ int      row;
    /* 0x1D4 */ u_char   pad1D4[0x1FC];
    /* 0x3D0 */ union {
        MenuList list[3];
        u_char   value;   /* the low byte of list[0].cur, which is all an
                             option value ever needs */
    } sel;
} MenuCtx;

extern MenuCtx *g_menu;

#endif
