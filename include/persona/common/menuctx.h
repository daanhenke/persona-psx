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
    /* 0x000 */ MenuList top;           /* the menu's own command cursor    */
    /* 0x010 */ MenuList status_page;   /* the member page's three stops    */
    /* 0x020 */ MenuList status_who;    /* the member the status menu is on */
    /* 0x030 */ MenuList unk030;
    /* 0x040 */ MenuList unk040;
    /* 0x050 */ MenuList unk050;
    /* 0x060 */ MenuList status_cmd;    /* skills, member pages, stock      */
    /* 0x070 */ MenuList skill_member;  /* the skills screen's member       */
    /* 0x080 */ MenuList skill_persona; /* one of the member's Personas     */
    /* 0x090 */ MenuList skill_spell;   /* and one of its spells            */
    /* 0x0A0 */ MenuList status_member; /* the status menu's party member   */
    /* 0x0B0 */ MenuList persona_cmd;   /* 0 equips the Persona, else views */
    /* 0x0C0 */ MenuList persona_slot;  /* which of the member's Personas   */
    /* 0x0D0 */ MenuList page;     /* the persona data view's page         */
    /* 0x0E0 */ MenuList stock;         /* the status menu's Persona stock  */
    /* 0x0F0 */ MenuList stock_release; /* the stock entry to let go of     */
    /* 0x100 */ MenuList unk100;
    /* 0x110 */ MenuList arcana_row;  /* the persona screen's arcana grid */
    /* 0x120 */ MenuList arcana_col;
    /* 0x130 */ MenuList unk130;
    /* 0x140 */ MenuList unk140;
    /* 0x150 */ u_char   pad150[0x20];
    /* 0x170 */ MenuList unk170;
    /* 0x180 */ u_char   pad180[0x40];
    /* 0x1C0 */ int      slot_base;
    /* 0x1C4 */ u_char   pad1C4[0xC];
    /* 0x1D0 */ int      row;
    /* 0x1D4 */ u_char   pad1D4[0xC];
    /* 0x1E0 */ MenuList member_list; /* the config tactics page's member */
    /* 0x1F0 */ u_char   pad1F0[0x30];
    /* 0x220 */ MenuList unk220;
    /* 0x230 */ MenuList unk230;
    /* 0x240 */ MenuList unk240;
    /* 0x250 */ MenuList unk250;
    /* 0x260 */ u_char   pad260[0xC0];
    /* 0x320 */ MenuList formation_cmd; /* 0 arranges the party, 1 presets  */
    /* 0x330 */ u_char   pad330[0x70];
    /* 0x3A0 */ MenuList grid[2];  /* the formation grid's row, then column  */
    /* 0x3C0 */ MenuList list[4];
} MenuCtx;

extern MenuCtx *g_menu;

/* Which step of a menu is running, and whether a held button still counts.
   menu.h declares them too. */
extern short  g_menu_subsel;
extern u_char g_menu_allow_hold;

#endif
