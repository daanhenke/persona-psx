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
    /* 0x004 */ u_char   pad004[0x9C];
    /* 0x0A0 */ MenuList status_member; /* the status menu's party member   */
    /* 0x0B0 */ MenuList persona_cmd;   /* 0 equips the Persona, else views */
    /* 0x0C0 */ MenuList persona_slot;  /* which of the member's Personas   */
    /* 0x0D0 */ MenuList page;     /* the persona data view's page         */
    /* 0x0E0 */ MenuList stock;         /* the status menu's Persona stock  */
    /* 0x0F0 */ MenuList stock_release; /* the stock entry to let go of     */
    /* 0x100 */ u_char   pad100[0x10];
    /* 0x110 */ MenuList arcana_row;  /* the persona screen's arcana grid */
    /* 0x120 */ MenuList arcana_col;
    /* 0x130 */ MenuList unk130;
    /* 0x140 */ MenuList unk140;
    /* 0x150 */ u_char   pad150[0x70];
    /* 0x1C0 */ int      slot_base;
    /* 0x1C4 */ u_char   pad1C4[0xC];
    /* 0x1D0 */ int      row;
    /* 0x1D4 */ u_char   pad1D4[0xC];
    /* 0x1E0 */ MenuList member_list; /* the config tactics page's member */
    /* 0x1F0 */ u_char   pad1F0[0x1B0];
    /* 0x3A0 */ MenuList grid[2];  /* the formation grid's row, then column  */
    /* 0x3C0 */ MenuList list[4];
} MenuCtx;

extern MenuCtx *g_menu;

/* Which step of a menu is running, and whether a held button still counts.
   menu.h declares them too. */
extern short  g_menu_subsel;
extern u_char g_menu_allow_hold;

#endif
