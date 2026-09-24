/* Persona 1 (JP) - the status menu.  ADV only.
 *   0x8006CA88 StatusMenuStep   0x8006CB7C StatusMenuOpen
 *   0x8006CFAC StatusStockScreen
 */
#include <decomp/types.h>

/* This unit's calls to SlotSetFlicker pass the slot unmasked. */
#define SLOT_FLICKER_INT

#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>
#include <persona/common/formation.h>
#include <persona/common/bg.h>
#include <persona/adv/personapage.h>
#include <persona/common/spell.h>

/* The stock screen's first step. */
#define STOCK_PICK 10

/* The status menu's commands. */
#define STATUS_SKILLS   0
#define STATUS_PERSONAS 1
#define STATUS_STOCK    2

extern short   g_menu_subsel;
extern short   g_menu_sel;
extern u_char  g_menu_blink;
extern u_short g_key_menu_close;
extern int     g_pad_pressed[];
extern short   g_stock_last;
extern u_char  g_fm_mark_def[];
extern short   g_fm_mark_pos[][2];
extern u_short g_menu_bg_rle[];
extern u_char  D_800B17E0[];
extern u_char  D_800B1D08[];
extern u_char  D_800B2330[];

extern void  DrawStatusHud(void);
extern void  BgBoxShow(void);
extern void  func_800782A4(int a, int b);
extern void  func_8008C23C(short member);
extern short PersonaStockCompact(void);
extern void  PersonaStockDraw(void);
extern void  func_80076CE0(void);
extern u_char MenuStepMember(int *sel, u_char last);
extern u_char CharTopEntry(short slot);
extern u_char PersonaTopSpell(short id);
extern void   BgPanelSet(short id, short x, short y);
extern void   func_8007AF78(void);
extern void   func_8007B288(short member);
extern void   func_8007B554(short member, short persona);
extern void   func_800768F0(void);
extern u_char D_800B12B8[];
extern u_char D_800B1EB8[];
extern u_char D_800B17E8[];
extern u_char ItemUsableAny(short item);
extern u_char ItemUsableOn(short slot, short item);
extern void   func_800946C4(short target, short caster, short spell);
extern void   MenuScreenDraw(void);
extern void   StatusPersonaPreview(void);

/* The two escape spells need no target. */
#define SPELL_ESCAPE_A 0x6F
#define SPELL_ESCAPE_B 0x73
#define TARGET_PARTY   4
extern u_char g_persona_list_rule[];
extern void   func_8007B6C0(u_char key, short *dst, int base);
extern void   DrawCharStatBars(Char *rec);

/* The glyph between the header's Persona and its bank. */
#define GLYPH_SEP 0xCD
/* The spell whose description the skills screen's panel shows. */
extern short  g_skill_help_spell;

extern void SoundPlaySeq(u_short slot, u_short seq, short vab);
extern void func_80077F8C(int a, int b);
extern void func_8007A62C(int a, int b);
extern void StatusPersonaPick(void);
extern void StatusPersonaView(void);
extern void StatusStockPick(void);
extern void StatusStockReleasePick(void);
extern void StatusStockReleaseConfirm(void);
extern void StatusStockView(void);

void StatusMenuOpen(void);
void StatusTopStep(void);
short StatusStockOpen();
void SkillMemberPick(void);
void SkillPersonaPick(void);
void SkillSpellPick(void);
void SkillTargetPick(void);
void StatusMemberPick(void);
void StatusPageLayout(void);
void StatusPageDraw(short member);
void StatusPageStep(void);

void StatusMenuStep(void)
{
    switch (g_menu_subsel) {
    case 0:
        StatusMenuOpen();
        g_menu_subsel++;
        break;
    case 1:
        StatusTopStep();
        break;
    case 2:
        SkillMemberPick();
        break;
    case 3:
        SkillPersonaPick();
        break;
    case 4:
        SkillSpellPick();
        break;
    case 5:
        SkillTargetPick();
        break;
    case 6:
        StatusMemberPick();
        break;
    case 7:
        StatusPageStep();
        break;
    case 8:
        StatusPersonaPick();
        break;
    case 9:
        StatusPersonaView();
        break;
    }
}

void StatusMenuOpen(void)
{
    func_800768F0();
    SlotSetAnim(0x2D, 0, 0, 0, 0, 0xC, 0, 0);
    func_80077F8C(1, 2);
    func_8007A62C(1, 6);
}

/* The status menu's command list, a frame: skills, the member pages, and
   the Persona stock. */
void StatusTopStep(void)
{
    int i;

    DrawStatusHud();
    if (MenuStepCursor(&g_menu->status_cmd)) {
        func_800782A4(1, 6);
    }
    if (InputCheckAcceptA(2)) {
        switch (g_menu->status_cmd.cur) {
        case STATUS_SKILLS:
            SlotClearAll();
            SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
            SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
            SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0xC, 0, 0);
            SlotInitTagged(g_fm_mark_def, 1, 0x42,
                           (g_fm_mark_pos + 1)[g_menu->skill_member.cur][0],
                           (g_fm_mark_pos + 1)[g_menu->skill_member.cur][1]);
            SlotSetFlicker(1, 1);
            g_menu_subsel++;
            break;
        case STATUS_PERSONAS:
            func_8008EDBC(7);
            SlotClearAll();
            TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
            TileMapDrawWindow(AT(g_tilemap0, 0, 7), 0x1A, 7, MAP_W);
            TileMapDrawBox(AT(g_tilemap0, 1, 8), 0x18, 5, MAP_W);
            TileMapBlitRle(g_menu_bg_rle, AT(g_tilemap0, 8, 0), MAP_W);
            TileMapWriteRow(str_cell_run, AT(g_tilemap2, 0, 3), 0x457, 6);
            for (i = 0; i < 3; i++) {
                TileMapWriteBar(AT(g_tilemap0, 2 + i, 10), 10);
                TileMapWriteBar(AT(g_tilemap0, 2 + i, 21), 10);
                *AT(g_tilemap2, 1 + i, 0) = 0x418 + i;
            }
            func_8008C23C(g_menu->status_member.cur);
            BgBoxShow();
            DrawStatusHud();
            SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
            SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
            SlotSetAnim(0x2D, 0, 0, 0, 0x60, 0xC, 0, 0);
            SlotInitTagged(g_fm_mark_def, 1, 0x42,
                           (g_fm_mark_pos + 1)[g_menu->status_member.cur][0],
                           (g_fm_mark_pos + 1)[g_menu->status_member.cur][1]);
            SlotSetFlicker(1, 1);
            g_menu_subsel += 5;
            break;
        case STATUS_STOCK:
            StatusStockScreen();
            break;
        }
    } else if (InputCheckAcceptB(2) || g_menu_allow_hold) {
        g_menu_blink = 0xFF;
        g_menu_sel = 0;
        g_menu_subsel = 0;
    }
}

/* The Persona stock screen: its own loop until a step leaves it. */
void StatusStockScreen(void)
{
    if (StatusStockOpen(1)) {
        return;
    }
    g_menu_subsel = STOCK_PICK;
loop:
    RunFrame();
    if (!g_menu_allow_hold && (g_key_menu_close & g_pad_pressed[0])) {
        g_menu_allow_hold = 1;
        SoundPlaySeq(0x18, 0, 1);
    }
    switch (g_menu_subsel) {
    case 0:
        return;
    case STOCK_PICK:
        StatusStockPick();
        break;
    case STOCK_PICK + 1:
        StatusStockReleasePick();
        break;
    case STOCK_PICK + 2:
        StatusStockReleaseConfirm();
        break;
    case STOCK_PICK + 3:
        StatusStockView();
        break;
    }
    goto loop;
}

/* The Persona stock list. With nothing in stock there is nothing to show:
   on the way in (`first`) the menu is left altogether. */
short StatusStockOpen(first)
    short first;
{
    int i;

    func_8008EDBC(0xA);
    SlotClearAll();
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x1E, 0x12, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 1, 1), 0x1C, 0x10, MAP_W);
    TileMapWriteRow(D_800B17E0, AT(g_tilemap1, 13, 2), 0x285, 6);
    for (i = 0; i < STOCK_ROWS; i++) {
        TileMapWriteBar(AT(g_tilemap0, 2 + i, 2), 0xB);
        TileMapWriteBar(AT(g_tilemap0, 2 + i, 13), 5);
        TileMapWriteBar(AT(g_tilemap0, 2 + i, 18), 10);
    }
    g_stock_last = PersonaStockCompact();
    PersonaStockDraw();
    TileMapWriteBar(AT(g_tilemap0, 15, 2), 10);
    SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x90, 0xC, 0, 0);
    if (g_stock_last == -1) {
        if (first) {
            func_80076CE0();
            g_menu_subsel = 0;
            return 1;
        }
        return 0;
    }
    SlotInitTagged(g_pdata_cursor_def, 1, 0x42, 0, 0);
    if (g_menu->stock.cur != g_stock_last + 1) {
        SlotSetPos(1, 0x42, 0x48, g_menu->stock.cur * 12 + 0x24);
    } else {
        SlotSetPos(1, 0x42, 0x48, 0xC0);
    }
    SlotSetFlicker(1, 1);
    return 0;
}

/* The skills screen's member marker, a frame. Accepting a member lays out
   their Personas and the first one's spells. */
void SkillMemberPick(void)
{
    DrawStatusHud();
    if (MenuStepMember(&g_menu->skill_member.cur, g_party_last)) {
        MenuListInit(&g_menu->skill_persona, 0, 0,
                     CharTopEntry(g_menu->skill_member.cur), 0x16);
        g_menu->skill_spell.cur = 0;
        SlotSetPos(1, 0x42, (g_fm_mark_pos + 1)[g_menu->skill_member.cur][0],
                   (g_fm_mark_pos + 1)[g_menu->skill_member.cur][1]);
    }
    if (InputCheckAcceptA(1)) {
        func_8008EDBC(6);
        func_8007AF78();
        func_8007B288(g_menu->skill_member.cur);
        MenuListInit(&g_menu->skill_persona, 0, 0,
                     CharTopEntry(g_menu->skill_member.cur), 0x16);
        func_8007B554(g_menu->skill_member.cur, g_menu->skill_persona.cur);
        SlotInitTagged(D_800B12B8, 2, 0x42, 0x58,
                       g_menu->skill_persona.cur * 12 + 0x54);
        SlotSetFlicker(2, 1);
        g_slot_cur = &g_slots[2];
        if (g_menu->skill_persona.hi == 0xFF) {
            g_slot_cur->attr |= SLOT_ATTR_HIDE;
        } else {
            g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        }
        g_menu_subsel++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_menu_subsel = 0;
    }
}

/* The member's Personas, a frame; the marker still moves between members.
   Accepting a Persona opens its spells, with the first one's description. */
void SkillPersonaPick(void)
{
    u_long   chars = (u_long)g_chars;
    Persona *personas = g_personas;
    Char    *c;
    int      i = g_menu->skill_member.cur;   /* the member before the step,
                                                 then a member, then a Persona */

    if (MenuStepCursor(&g_menu->skill_member)) {
        if (i != g_menu->skill_member.cur) {
            g_menu->skill_spell.cur = 0;
            MenuListInit(&g_menu->skill_persona, 0, 0,
                         CharTopEntry(g_menu->skill_member.cur), 0x16);
            func_8007B288(g_menu->skill_member.cur);
            func_8007B554(g_menu->skill_member.cur, g_menu->skill_persona.cur);
            SlotSetPos(1, 0x42,
                       (g_fm_mark_pos + 1)[g_menu->skill_member.cur][0],
                       (g_fm_mark_pos + 1)[g_menu->skill_member.cur][1]);
            g_slot_cur = &g_slots[2];
            if (g_menu->skill_persona.hi == 0xFF) {
                g_slot_cur->attr |= SLOT_ATTR_HIDE;
            } else {
                g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
            }
        }
    } else if (g_menu->skill_persona.hi != 0xFF &&
               MenuStepCursor(&g_menu->skill_persona)) {
        g_menu->skill_spell.cur = 0;
        func_8007B554(g_menu->skill_member.cur, g_menu->skill_persona.cur);
    }
    SlotSetPos(2, 0x42, 0x58, g_menu->skill_persona.cur * 12 + 0x54);
    DrawStatusHud();
    if (InputCheckAcceptA(1)) {
        if (g_menu->skill_persona.hi != 0xFF) {
            i = g_party_at[g_menu->skill_member.cur];
            c = (Char *)(i * sizeof(Char) + chars);
            i = c->list[g_menu->skill_persona.cur];
            MenuListInit(&g_menu->skill_spell, 0, 0, PersonaTopSpell(i), 0x16);
            if (i != 0xFF && !c->blocked) {
                g_skill_help_spell = personas[i].spell[g_menu->skill_spell.cur];
            } else {
                g_skill_help_spell = 0;
            }
            BgPanelSet(g_skill_help_spell, 0x3C, 0xE);
            SlotInitTagged(g_pdata_cursor_def, 3, 0x42, 0xC8,
                           g_menu->skill_spell.cur * 12 + 0x24);
            SlotInitTagged(D_800B1EB8, 0x2E, 0x24, 0x3A, 0xC);
            if (g_menu->skill_spell.hi == 0xFF) {
                g_slot_cur = &g_slots[3];
                g_slot_cur->attr |= SLOT_ATTR_HIDE;
            }
            SlotSetFlicker(1, 0);
            SlotSetFlicker(2, 0);
            SlotSetFlicker(3, 1);
            g_menu_subsel++;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        func_800768F0();
        SlotClear(2);
        SlotClear(0x2F);
        SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
        SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
        SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0xC, 0, 0);
        SlotInitTagged(g_fm_mark_def, 1, 0x42,
                       (g_fm_mark_pos + 1)[g_menu->skill_member.cur][0],
                       (g_fm_mark_pos + 1)[g_menu->skill_member.cur][1]);
        SlotSetFlicker(1, 1);
        g_menu_subsel--;
    }
}

/* The Persona's spells, a frame: the description panel follows the cursor.
   Accepting a spell the field allows, with the SP for it, either marks every
   member (a spell for the whole party) or puts the target cursor on the
   first; the two escape spells need no target. */
void SkillSpellPick(void)
{
    u_long chars = (u_long)g_chars;
    u_long personas = (u_long)g_personas;
    int    who = g_party_at[g_menu->skill_member.cur];
    int    i = ((Char *)(who * sizeof(Char) + chars))->list[g_menu->skill_persona.cur];
    int    prev;
    int    slot;
    Persona *p;

    prev = g_menu->skill_spell.cur;
    if (g_menu->skill_spell.hi != 0xFF && MenuStepCursor(&g_menu->skill_spell) &&
        prev != g_menu->skill_spell.cur) {
        if (i != 0xFF) {
            g_skill_help_spell = ((Persona *)(i * sizeof(Persona) + personas))
                                     ->spell[g_menu->skill_spell.cur];
        } else {
            g_skill_help_spell = 0;
        }
        BgPanelSet(g_skill_help_spell, 0x3C, 0xE);
        SlotSetPos(3, 0x42, 0xC8, g_menu->skill_spell.cur * 12 + 0x24);
    }
    DrawStatusHud();
    MsgStep();
    if (InputCheckAcceptA(1)) {
        p = (Persona *)(i * sizeof(Persona) + personas);
        g_skill_help_spell = p->spell[g_menu->skill_spell.cur];
        if (ItemUsableAny(g_skill_help_spell) &&
            ((Char *)(who * sizeof(Char) + chars))->sp >= p->sp_cost) {
            SlotSetFlicker(3, 0);
            if ((u_short)g_skill_help_spell != SPELL_ESCAPE_A &&
                (u_short)g_skill_help_spell != SPELL_ESCAPE_B) {
                if (g_spell_data[(u_short)g_skill_help_spell].target == TARGET_PARTY) {
                    for (i = 0; i <= g_party_last; i++) {
                        slot = i + 4;
                        SlotInitTagged(g_fm_mark_def, slot, 0x42,
                                       (g_fm_mark_pos + 1)[i][0],
                                       (g_fm_mark_pos + 1)[i][1]);
                        SlotSetFlicker(slot, 1);
                        g_slot_cur = &g_slots[i + 4];
                        g_slot_cur->flicker = 0;
                    }
                } else {
                    MenuListInit(&g_menu->list[1], 0, 0, g_party_last, 0x10);
                    SlotInitTagged(g_fm_mark_def, 4, 0x42, g_fm_mark_pos[1][0],
                                   g_fm_mark_pos[1][1]);
                    SlotSetFlicker(4, 1);
                }
            }
            g_slot_cur = &g_slots[1];
            g_slot_cur->attr |= SLOT_ATTR_HIDE;
            g_menu_subsel++;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        SlotClear(3);
        SlotClear(0x2E);
        SlotSetFlicker(1, 1);
        SlotSetFlicker(2, 1);
        g_bg_shown ^= 0x10;
        g_menu_subsel--;
    }
}

/* Casting the spell, a frame: the target cursor walks the party, unless the
   spell takes them all. Casting pays the Persona's SP cost; the screen stays
   on the spell while the member can afford another. */
void SkillTargetPick(void)
{
    u_long   chars = (u_long)g_chars;
    u_long   personas = (u_long)g_personas;
    int      who = g_party_at[g_menu->skill_member.cur];
    int      p = ((Char *)(who * sizeof(Char) + chars))->list[g_menu->skill_persona.cur];
    int      i;
    Char    *c;
    Persona *pp;

    DrawStatusHud();
    MsgStep();
    if ((u_short)g_skill_help_spell == SPELL_ESCAPE_A ||
        (u_short)g_skill_help_spell == SPELL_ESCAPE_B) {
        goto cast;
    }
    if (g_spell_data[(u_short)g_skill_help_spell].target != TARGET_PARTY &&
        MenuStepMember(&g_menu->list[1].cur, g_party_last)) {
        SlotSetPos(4, 0x42, (g_fm_mark_pos + 1)[g_menu->list[1].cur][0],
                   (g_fm_mark_pos + 1)[g_menu->list[1].cur][1]);
    }
    if (InputCheckAcceptA(1)) {
        if (g_spell_data[(u_short)g_skill_help_spell].target == TARGET_PARTY) {
            for (i = 0; i <= g_party_last; i++) {
                func_800946C4(i, g_menu->skill_member.cur, g_skill_help_spell);
            }
        } else {
            if (!ItemUsableOn(g_menu->list[1].cur,
                              (u_short)g_skill_help_spell)) {
                return;
            }
        cast:
            func_800946C4(g_menu->list[1].cur, g_menu->skill_member.cur,
                          g_skill_help_spell);
        }
        c = (Char *)(who * sizeof(Char) + chars);
        pp = (Persona *)(p * sizeof(Persona) + personas);
        c->sp -= pp->sp_cost;
        MenuScreenDraw();
        func_8007AF78();
        func_8007B288(g_menu->skill_member.cur);
        func_8007B554(g_menu->skill_member.cur, g_menu->skill_persona.cur);
        if (!ItemUsableAny(g_skill_help_spell) || c->sp < pp->sp_cost) {
            goto back;
        }
        return;
    } else if (!InputCheckAcceptB(1) && !g_menu_allow_hold) {
        return;
    }
back:
    SlotSetFlicker(3, 1);
    for (who = 0; who < 5; who++) {
        SlotClear(who + 4);
    }
    g_slot_cur = &g_slots[1];
    g_slot_cur->attr ^= SLOT_ATTR_HIDE;
    g_menu_subsel--;
}

/* The member pages' marker, a frame; accepting a member lays out their
   page. */
void StatusMemberPick(void)
{
    DrawStatusHud();
    if (MenuStepMember(&g_menu->status_member.cur, g_party_last)) {
        func_8008C23C(g_menu->status_member.cur);
        SlotSetPos(1, 0x42, (g_fm_mark_pos + 1)[g_menu->status_member.cur][0],
                   (g_fm_mark_pos + 1)[g_menu->status_member.cur][1]);
    }
    if (InputCheckAcceptA(1)) {
        StatusPageLayout();
        StatusPageDraw(g_menu->status_member.cur);
        SlotInitTagged(g_pdata_cursor_def, 1, 0x42, 0x58,
                       g_menu->persona_cmd.cur * 12 + 0x24);
        SlotInitTagged(g_pdata_bottom_def, PAGE_BOTTOM_SLOT, 0x50, 0x48, 0x9C);
        SlotSetFlicker(PAGE_BOTTOM_SLOT, 1);
        SlotSetFlicker(1, 1);
        g_menu_subsel++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_menu_subsel = 0;
    }
}

/* A member's page: the five stats, the name, the two contact values and the
   three Persona rows, the active one repeated in the header in the bright
   glyph bank. An empty row, or every row while the list is blocked, shows the
   dashed rule. */
void StatusPageDraw(member)
    short member;
{
    int      i = g_party_at[member];   /* the member, then a Persona */
    Char    *c = &g_chars[i];
    Persona *personas = g_personas;
    int      row;
    int      bank;
    u_short  n;

    TileMapFillRect(AT(g_tilemap1, 4, 11), 0, 10, 3, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 10, 12), 0, 8, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 3, 25), 0, 10, 7, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 10, 20), 0, 12, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 16, 14), 0, 3, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 16, 24), 0, 3, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 1, 31), 0, 4, 2, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 11, 23), 0, 2, 5, MAP_W);
    n = FormatDecimal(g_chars[i].stat[0], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 11, 24), GLYPH_DIGIT0, n);
    n = FormatDecimal(g_chars[i].stat[1], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 12, 24), GLYPH_DIGIT0, n);
    n = FormatDecimal(g_chars[i].stat[2], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 13, 24), GLYPH_DIGIT0, n);
    n = FormatDecimal(g_chars[i].stat[3], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 14, 24), GLYPH_DIGIT0, n);
    n = FormatDecimal(g_chars[i].stat[4], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 15, 24), GLYPH_DIGIT0, n);
    TileMapWriteRow(c->name, AT(g_tilemap1, 10, 12), 0, 8);
    n = FormatDecimal(g_chars[i].unk3A, g_hud_digits, 3);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 16, 16), GLYPH_DIGIT0, n);
    n = FormatDecimal(g_chars[i].unk3C, g_hud_digits, 3);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 16, 26), GLYPH_DIGIT0, n);

    for (row = 0; row < 3; row++) {
        i = c->list[row];
        bank = 0;
        if (i != 0xFF && !c->blocked) {
            if (c->entry == row) {
                bank = 3;
                *AT(g_tilemap1, 10, 20) = GLYPH_SEP;
                func_8007B6C0(personas[i].key, AT(g_tilemap1, 10, 22), 0);
            }
            func_8007B6C0(personas[i].key, AT(g_tilemap1, 4 + row, 11),
                          bank * 0xD7);
        } else {
            TileMapWriteRow(g_persona_list_rule, AT(g_tilemap1, 4 + row, 12),
                            0xD7, 8);
        }
    }
    DrawCharStatBars(c);
}

/* One of a member's Persona slots on their page: the Persona's name in the
   header, its level and SP cost, its two contact values and its seven spells.
   An empty slot shows zeroes, drawn from the counter itself. */
void StatusSlotDraw(member, slot)
    short member;
    short slot;
{
    int     i = g_party_at[member];   /* the member, then the Persona */
    Char   *c = &g_chars[i];
    u_long  personas = (u_long)g_personas;
    int     n;

    TileMapFillRect(AT(g_tilemap1, 3, 25), 0, 10, 7, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 10, 22), 0, 10, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 16, 14), 0, 3, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 16, 24), 0, 3, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 1, 32), 0, 3, 2, MAP_W);
    i = c->list[slot];
    if (i != 0xFF) {
        func_8007B6C0(g_personas[i].key, AT(g_tilemap1, 10, 22), 0);
        n = FormatDecimal(g_personas[i].level, g_hud_digits, 2);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 1, 34), GLYPH_DIGIT0, n);
        n = FormatDecimal(g_personas[i].sp_cost, g_hud_digits, 3);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 2, 34), GLYPH_DIGIT0, n);
        n = FormatDecimal(g_personas[i].unk10, g_hud_digits, 3);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 16, 16), GLYPH_DIGIT0, n);
        n = FormatDecimal(g_personas[i].unk12, g_hud_digits, 3);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 16, 26), GLYPH_DIGIT0, n);
    } else {
        n = 0;
        TileMapWriteRowRev((u_char *)&n, AT(g_tilemap1, 1, 34), GLYPH_DIGIT0, 1);
        TileMapWriteRowRev((u_char *)&n, AT(g_tilemap1, 2, 34), GLYPH_DIGIT0, 1);
        TileMapWriteRowRev((u_char *)&n, AT(g_tilemap1, 16, 16), GLYPH_DIGIT0, 1);
        TileMapWriteRowRev((u_char *)&n, AT(g_tilemap1, 16, 26), GLYPH_DIGIT0, 1);
    }
    for (n = 0; n < PERSONA_SPELLS; n++) {
        DrawSpellName(((Persona *)(i * sizeof(Persona) + personas))->spell[n],
                      AT(g_tilemap1, n + 3, 25), 0, 1);
    }
}

/* The member's page for the preview, from a copy of their record: the
   active Persona's name, level, spell slots, SP cost and contact values, the
   member's five stats as the Persona leaves them, and its seven spells. */
void StatusPreviewDraw(Char *c)
{
    u_long personas = (u_long)g_personas;
    int    i;
    int    n;

    TileMapFillRect(AT(g_tilemap1, 3, 25), 0, 10, 7, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 10, 22), 0, 10, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 16, 14), 0, 3, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 16, 24), 0, 3, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 1, 31), 0, 4, 2, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 11, 23), 0, 2, 5, MAP_W);
    i = c->list[c->entry];
    if (i != 0xFF) {
        func_8007B6C0(g_personas[i].key, AT(g_tilemap1, 10, 22), 0);
        n = FormatDecimal(g_personas[i].level, g_hud_digits, 2);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 1, 32), GLYPH_DIGIT0, n);
        *AT(g_tilemap1, 1, 33) = GLYPH_SEP;
        FormatDecimal(g_personas[i].slots, g_hud_digits, 1);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 1, 34), GLYPH_DIGIT0, 1);
        n = FormatDecimal(g_personas[i].sp_cost, g_hud_digits, 3);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 2, 33), GLYPH_DIGIT0, n);
        n = FormatDecimal(g_personas[i].unk10, g_hud_digits, 3);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 16, 16), GLYPH_DIGIT0, n);
        n = FormatDecimal(g_personas[i].unk12, g_hud_digits, 3);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 16, 26), GLYPH_DIGIT0, n);
        n = FormatDecimal(c->stat[0], g_hud_digits, 2);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 11, 24), GLYPH_DIGIT0, n);
        n = FormatDecimal(c->stat[1], g_hud_digits, 2);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 12, 24), GLYPH_DIGIT0, n);
        n = FormatDecimal(c->stat[2], g_hud_digits, 2);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 13, 24), GLYPH_DIGIT0, n);
        n = FormatDecimal(c->stat[3], g_hud_digits, 2);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 14, 24), GLYPH_DIGIT0, n);
        n = FormatDecimal(c->stat[4], g_hud_digits, 2);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 15, 24), GLYPH_DIGIT0, n);
    } else {
        n = 0;
        TileMapWriteRowRev((u_char *)&n, AT(g_tilemap1, 1, 34), GLYPH_DIGIT0, 1);
        TileMapWriteRowRev((u_char *)&n, AT(g_tilemap1, 2, 34), GLYPH_DIGIT0, 1);
        TileMapWriteRowRev((u_char *)&n, AT(g_tilemap1, 16, 16), GLYPH_DIGIT0, 1);
        TileMapWriteRowRev((u_char *)&n, AT(g_tilemap1, 16, 26), GLYPH_DIGIT0, 1);
    }
    for (n = 0; n < PERSONA_SPELLS; n++) {
        DrawSpellName(((Persona *)(i * sizeof(Persona) + personas))->spell[n],
                      AT(g_tilemap1, n + 3, 25), 0, 1);
    }
}

/* A member's status page, laid out: the two commands, the member's three
   Persona rows, the stats and the spell rows. */
void StatusPageLayout(void)
{
    int     i;
    u_char *arc;

    func_8008EDBC(8);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(AT(g_tilemap0, 0, 7), 0x1E, 0x13, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 1, 8), 0x1C, 0x11, MAP_W);
    for (i = 0; i < 2; i++) {
        TileMapWriteBar(AT(g_tilemap0, 2 + i, 11), 10);
    }
    for (i = 0; i < 3; i++) {
        TileMapWriteBar(AT(g_tilemap0, 5 + i, 11), 10);
        *AT(g_tilemap1, 4 + i, 10) = 0x418 + i;
    }
    for (i = 0; i < 7; i++) {
        TileMapWriteBar(AT(g_tilemap0, 4 + i, 25), 10);
    }
    for (i = 0; i < 5; i++) {
        TileMapWriteBar(AT(g_tilemap0, 12 + i, 9), 0x1A);
    }
    TileMapWriteRow(D_800B17E8, AT(g_tilemap1, 1, 13), 0, 6);
    TileMapWriteRow(D_800B17E8 + 6, AT(g_tilemap1, 2, 13), 0, 6);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 3, 13), 0x457, 6);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 1, 25), 0x36F, 5);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 2, 28), 0x37A, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 16, 9), 0x3CB, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 16, 19), 0x3D1, 3);
    arc = &D_800B92A0[0x4C];
    TileMapWriteRow(arc, AT(g_tilemap1, 11, 19), 0, 3);
    TileMapWriteRow(arc + 3, AT(g_tilemap1, 12, 19), 0, 3);
    TileMapWriteRow(arc + 6, AT(g_tilemap1, 13, 19), 0, 3);
    TileMapWriteRow(arc + 9, AT(g_tilemap1, 14, 19), 0, 3);
    TileMapWriteRow(arc + 12, AT(g_tilemap1, 15, 19), 0, 3);
}

/* A member's page, a frame: the marker moves between members and the cursor
   between the two commands. Accepting opens the member's Personas, when they
   have any; backing out returns to the member list. */
void StatusPageStep(void)
{
    int i;

    if (MenuStepCursor(&g_menu->status_member)) {
        StatusPageDraw(g_menu->status_member.cur);
    } else if (MenuStepCursor(&g_menu->persona_cmd)) {
        SlotSetPos(1, 0x42, 0x58, g_menu->persona_cmd.cur * 12 + 0x24);
        g_menu->persona_slot.cur = 0;
    }
    if (InputCheckAcceptA(1)) {
        i = CharTopEntry(g_menu->status_member.cur);
        if (i != 0xFF) {
            MenuListInit(&g_menu->persona_slot, 0, 0, i, 0x16);
            StatusPersonaPreview();
            SlotInitTagged(g_pdata_cursor_def, 2, 0x42, 0x58,
                           g_menu->persona_slot.cur * 12 + 0x48);
            SlotSetFlicker(1, 0);
            SlotSetFlicker(2, 1);
            g_menu_subsel++;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        func_800768F0();
        func_8008EDBC(7);
        SlotClearAll();
        TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
        TileMapDrawWindow(AT(g_tilemap0, 0, 7), 0x1A, 7, MAP_W);
        TileMapDrawBox(AT(g_tilemap0, 1, 8), 0x18, 5, MAP_W);
        TileMapBlitRle(g_menu_bg_rle, AT(g_tilemap0, 8, 0), MAP_W);
        TileMapWriteRow(str_cell_run, AT(g_tilemap2, 0, 3), 0x457, 6);
        for (i = 0; i < 3; i++) {
            TileMapWriteBar(AT(g_tilemap0, 2 + i, 10), 10);
            TileMapWriteBar(AT(g_tilemap0, 2 + i, 21), 10);
            *AT(g_tilemap2, 1 + i, 0) = 0x418 + i;
        }
        func_8008C23C(g_menu->status_member.cur);
        BgBoxShow();
        DrawStatusHud();
        SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
        SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
        SlotSetAnim(0x2D, 0, 0, 0, 0x60, 0xC, 0, 0);
        SlotInitTagged(g_fm_mark_def, 1, 0x42,
                       (g_fm_mark_pos + 1)[g_menu->status_member.cur][0],
                       (g_fm_mark_pos + 1)[g_menu->status_member.cur][1]);
        SlotSetFlicker(1, 1);
        g_menu_subsel--;
    }
}
