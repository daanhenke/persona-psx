/* Persona 1 (JP) - the save screen.  ADV only.
 *   0x80099DE8 SaveScreenStep     0x80099E9C SaveScreenOpen
 *   0x8009A0A4 SaveTopStep        0x8009A5AC SaveQuitStep
 *   0x8009A72C SavePortStep       0x8009A8C0 SaveSlotStep
 *   0x8009ADC4 SaveConfirmStep    0x8009B220 SaveOfferFormat
 *   0x8009B3C8 SaveScanSlots      0x8009B4C4 SaveSlotsOpen
 *   0x8009B61C SaveSlotsDraw      0x8009B9E4 SaveMessage
 *   0x8009BAD0 SavePortOpen
 *
 * The facility host's kind 10: a two-line prompt, then the memory card port,
 * then one of the seven save slots, then a yes/no before the save is written
 * with main's card routines. Each card check reads the port's state with
 * CardLoad, which answers 1 (an error, asked twice before it is believed),
 * 2 (no card), 3 (the card was swapped), 4 (not formatted) or 0 (ready).
 * A save in two particular rooms also ends the scene.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/bg.h>
#include <persona/adv/personapage.h>

#define STEP_DONE 0xFF

/* A card state: what CardLoad answers. */
#define CARD_READY    0
#define CARD_ERROR    1
#define CARD_NONE     2
#define CARD_SWAPPED  3
#define CARD_UNFORMAT 4

/* A slot of the list: which kind of save it holds, or none. */
#define SAVE_EMPTY 0xFF

#define SAVE_SLOTS 7

/* The rooms whose save point ends the scene. */
#define MAP_SAVE_A 0x347
#define MAP_SAVE_B 0x357
#define MAP_SAVE_C 0x36B

/* What CardScanSaves reads for each save: the list shows the hero's name,
   level and time played. */
typedef struct {
    u_char name[8];
    u_char level;
    u_char hours;
    u_char minutes;
    u_char rest[0xF];
} SaveSummary;                      /* 0x1A bytes */

extern short       g_persona_data_step;
extern u_char      g_menu_allow_hold;
extern int         g_pad_pressed[];
extern u_char      g_facility_leave;
extern u_char      g_msg_answer;
extern u_short     g_prev_map;
extern short       g_card_state_a;
extern short       g_card_state_b;
extern short       g_save_kinds[SAVE_SLOTS];
extern SaveSummary g_save_list[2][SAVE_SLOTS];
extern u_char     *g_save_msgs[];
extern u_char      g_save_port_msg[];
extern u_char      g_save_quit_msg[];
extern u_char      g_save_port_msg2[];
extern u_char      g_save_format_ask[];
extern u_char      g_save_format_msg[];
extern u_char      g_save_busy_msg[];
extern u_char      g_save_slot_cur_def[];
extern u_char      g_fm_prompt_cur_def[];
extern u_char      g_fm_hint_def[];
extern u_char      g_fm_hint2_def[];
extern u_char      D_800B1D08[];
extern u_char      D_800B2330[];

extern int  CardLoad(u_char chan);
extern int  CardCheckFree(u_char chan);
extern int  CardFormat(u_char chan);
extern int  CardScanSaves(u_char chan, u_char kind, SaveSummary *out);
extern int  CardSave(u_char chan, u_char slot, u_char kind);
extern int  CardDeleteFile(u_char chan, u_char slot, u_char kind);
extern void CinemaOpen(short plain);
extern void CinemaClose(short plain);
extern void func_800AB1EC(void);

void SaveScreenOpen(void);
void SaveTopStep(void);
void SaveQuitStep(void);
void SavePortStep(void);
void SaveSlotStep(void);
void SaveConfirmStep(void);
short SaveOfferFormat(short keep);
void SaveScanSlots(short chan);
void SaveSlotsOpen(void);
void SaveSlotsDraw(void);
void SaveMessage(short n, short keep);
void SavePortOpen(void);

#define MSG_WINDOW(height)                                                         \
    g_bg_layers[4].x = 0x28;                                                  \
    g_bg_layers[4].y = 0xA4;                                                  \
    g_bg_layers[4].w = 0xF0;                                                  \
    g_bg_layers[4].h = (height)

void SaveScreenStep(void)
{
    switch (g_persona_data_step) {
    case 0:
        SaveScreenOpen();
        g_persona_data_step++;
        break;
    case 1:
        SaveTopStep();
        break;
    case 2:
        SaveQuitStep();
        break;
    case 3:
        SavePortStep();
        break;
    case 4:
        SaveSlotStep();
        break;
    case 5:
        SaveConfirmStep();
        break;
    }
}

void SaveScreenOpen(void)
{
    g_bg_shown = 0;
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    SlotClearAll();
    SlotInitTagged(D_800B1D08, 0x3C, 8, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 7, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0xC0, 0x30, 0, 0);
    SlotInitTagged(g_fm_prompt_cur_def, 3, 0x23, 0x30,
                   g_menu->unk2E0.cur * 16 + 0x32);
    SlotInitTagged(g_fm_hint_def, 9, 0x24, 0x30, 0x30);
    SlotInitTagged(g_fm_hint_def, 0xA, 0x24, 0x30, 0x40);
    SlotInitTagged(g_fm_hint2_def, 0xC, 0x22, 0x30, 0x30);
    SlotInitTagged(g_fm_hint2_def, 0xD, 0x22, 0x30, 0x40);
    SlotSetAnim(0xC, 0, 0, 0, 0, 0x10, 0, 0);
    SlotSetAnim(0xD, 0, 0, 0, 0x30, 0x10, 0, 0);
    SlotSetFlicker(3, 1);
}

void SaveTopStep(void)
{
    int state;

    MenuStepCursor(&g_menu->unk2E0);
    SlotSetPos(3, 0x23, 0x30, g_menu->unk2E0.cur * 16 + 0x32);
    if (InputCheckAcceptA(1)) {
        if (g_menu->unk2E0.cur != 0) {
            func_8008EDBC(0x20);
            TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
            TileMapDrawWindow(g_tilemap0, 0x1C, 6, MAP_W);
            TileMapDrawBox(g_tilemap0 + MAP_W + 1, 0x1A, 4, MAP_W);
            BgMapInit(g_save_port_msg, 0);
            g_bg_layers[4].x = 0x48;
            g_bg_layers[4].y = 0x2C;
            g_bg_layers[4].w = 0xF0;
            g_bg_layers[4].h = 0x20;
            g_bg_shown |= 0x10;
            MenuListInit(&g_menu->unk2F0, 1, 0, 1, 0x1E);
            SlotInitTagged(g_fm_prompt_cur_def, 3, 0x23, 0xE8, 0x8A);
            SlotInitTagged(g_fm_hint_def, 8, 0x24, 0xE8, 0x68);
            SlotInitTagged(g_fm_hint_def, 9, 0x24, 0xE8, 0x78);
            SlotInitTagged(g_fm_hint_def, 0xA, 0x24, 0xE8, 0x88);
            SlotInitTagged(g_fm_hint2_def, 0xB, 0x22, 0xE8, 0x68);
            SlotInitTagged(g_fm_hint2_def, 0xC, 0x22, 0xE8, 0x78);
            SlotInitTagged(g_fm_hint2_def, 0xD, 0x22, 0xE8, 0x88);
            SlotSetAnim(0xC, 0, 0, 0, 0x30, 0, 0, 0);
            SlotSetAnim(0xD, 0, 0, 0, 0x60, 0, 0, 0);
            SlotSetFlicker(3, 1);
            g_persona_data_step += 1;
            return;
        }
        VSync(0);
        g_card_state_a = CardLoad(0);
        g_card_state_b = CardLoad(1);
        if (g_card_state_a == CARD_NONE && g_card_state_b == g_card_state_a) {
            SaveMessage(0, 0);
            return;
        }
        func_800AB1EC();
        MenuListInit(&g_menu->unk2C0, 0, 0, 6, 0x1E);
        /* Only one port has a card: the cursor is held on it. */
        if (g_card_state_a != CARD_NONE) {
            if (g_card_state_b != CARD_NONE) {
                MenuListInit(&g_menu->unk2D0, 0, 0, 1, 0x1E);
                SavePortOpen();
                g_persona_data_step += 2;
                return;
            }
            if (g_card_state_a != g_card_state_b) {
                MenuListInit(&g_menu->unk2D0, 0, 0, 0, 0x1E);
                goto scan;
            }
        }
        MenuListInit(&g_menu->unk2D0, 1, 1, 1, 0x1E);
    scan:
        SaveScanSlots(g_menu->unk2D0.cur);
        SaveSlotsOpen();
        g_persona_data_step += 3;
        state = CardLoad(g_menu->unk2D0.cur);
        switch (state) {
        case CARD_ERROR:
            VSync(0);
            if (CardLoad(g_menu->unk2D0.cur) != state) {
                goto scan;
            }
            SaveMessage(3, 0);
            g_persona_data_step = 0;
            return;
        case CARD_NONE:
            SaveMessage(0, 0);
            g_persona_data_step = 0;
            return;
        case CARD_UNFORMAT:
            if (SaveOfferFormat(0)) {
                g_persona_data_step -= 3;
                goto scan;
            }
            g_persona_data_step = 0;
            return;
        }
        return;
    }
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_persona_data_step = STEP_DONE;
    }
}

void SaveQuitStep(void)
{
    MenuStepCursor(&g_menu->unk2F0);
    SlotSetPos(3, 0x23, 0xE8, g_menu->unk2F0.cur * 16 + 0x7A);
    MsgStep();
    if (InputCheckAcceptA(1)) {
        if (g_menu->unk2F0.cur == 0) {
            TileMapDrawWindow(g_tilemap0, 0x1C, 8, MAP_W);
            TileMapDrawBox(g_tilemap0 + MAP_W + 1, 0x1A, 6, MAP_W);
            func_800AB1EC();
            BgMapInit(g_save_quit_msg, 0);
            g_bg_layers[4].x = 0x48;
            g_bg_layers[4].y = 0x2C;
            g_bg_layers[4].w = 0xF0;
            g_bg_layers[4].h = 0x30;
            goto wait;
        more:
            MsgStep();
        wait:
            RunFrame();
            if (!g_pad_pressed[0]) {
                goto more;
            }
        }
        g_persona_data_step = STEP_DONE;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        SaveScreenOpen();
        g_persona_data_step -= 1;
    }
}

void SavePortStep(void)
{
    MenuStepCursor(&g_menu->unk2D0);
    SlotSetPos(3, 0x23, 0xE8, g_menu->unk2D0.cur * 16 + 0x6A);
    MsgStep();
    if (InputCheckAcceptA(1)) {
    scan:
        SaveScanSlots(g_menu->unk2D0.cur);
        SaveSlotsOpen();
        VSync(0);
        switch (CardLoad(g_menu->unk2D0.cur)) {
        case CARD_READY:
            g_persona_data_step += 1;
            return;
        case CARD_ERROR:
            VSync(0);
            if (CardLoad(g_menu->unk2D0.cur) != CARD_ERROR) {
                goto scan;
            }
            SaveMessage(3, 0);
            break;
        case CARD_NONE:
            SaveMessage(0, 0);
            break;
        case CARD_SWAPPED:
            goto scan;
        case CARD_UNFORMAT:
            if (SaveOfferFormat(0)) {
                goto scan;
            }
            break;
        }
        SavePortOpen();
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_persona_data_step = 0;
    }
}

void SaveSlotStep(void)
{
    int ends;
    int state;

    MenuStepCursor(&g_menu->unk2C0);
    SlotSetPos(4, 0x42, 0x48, g_menu->unk2C0.cur * 24 + 0x24);
    if (InputCheckAcceptA(1)) {
        ends = 0;
    retry:
        switch (g_prev_map) {
        case MAP_SAVE_A:
        case MAP_SAVE_B:
        case MAP_SAVE_C:
            ends = 1;
            break;
        }
        if (g_save_kinds[g_menu->unk2C0.cur] == 0 && ends == 1) {
            SaveMessage(10, 0);
            return;
        }
        if (g_save_kinds[g_menu->unk2C0.cur] == 1 && ends == 0) {
            SaveMessage(9, 0);
            return;
        }
        if (g_save_kinds[g_menu->unk2C0.cur] == SAVE_EMPTY) {
            VSync(0);
            if (CardCheckFree(g_menu->unk2D0.cur)) {
                goto check;
            }
        }
    confirm:
        MenuListInit(&g_menu->unk2B0, 0, 0, 1, 0x1E);
        SlotInitTagged(g_fm_prompt_cur_def, 3, 0x23, 0xF8, 0xBA);
        SlotInitTagged(g_fm_hint_def, 8, 0x24, 0xF8, 0xA8);
        SlotInitTagged(g_fm_hint_def, 9, 0x24, 0xF8, 0xB8);
        SlotInitTagged(g_fm_hint_def, 0xA, 0x24, 0xF8, 0xC8);
        SlotInitTagged(g_fm_hint2_def, 0xB, 0x22, 0xF8, 0xA8);
        SlotInitTagged(g_fm_hint2_def, 0xC, 0x22, 0xF8, 0xB8);
        SlotInitTagged(g_fm_hint2_def, 0xD, 0x22, 0xF8, 0xC8);
        SlotSetAnim(0xC, 0, 0, 0, 0x30, 0, 0, 0);
        SlotSetAnim(0xD, 0, 0, 0, 0x60, 0, 0, 0);
        SlotSetFlicker(4, 0);
        SlotSetFlicker(3, 1);
        g_persona_data_step += 1;
        return;
    check:
        VSync(0);
        state = CardLoad(g_menu->unk2D0.cur);
        switch (state) {
        case CARD_ERROR:
            VSync(0);
            if (CardLoad(g_menu->unk2D0.cur) == state) {
                SaveMessage(3, 0);
                SaveScanSlots(g_menu->unk2D0.cur);
                SaveSlotsOpen();
                return;
            }
            /* fall through */
        case CARD_SWAPPED:
            SaveScanSlots(g_menu->unk2D0.cur);
            ends = 0;
            SaveSlotsOpen();
            goto retry;
        case CARD_NONE:
            SaveMessage(0, 0);
            SaveScanSlots(g_menu->unk2D0.cur);
            SaveSlotsOpen();
            return;
        case CARD_UNFORMAT:
            if (SaveOfferFormat(0)) {
                goto confirm;
            }
            return;
        default:
            SaveScanSlots(g_menu->unk2D0.cur);
            SaveSlotsOpen();
            if (g_save_kinds[g_menu->unk2C0.cur] != SAVE_EMPTY) {
                goto confirm;
            }
            VSync(0);
            if (CardCheckFree(g_menu->unk2D0.cur)) {
                SaveMessage(1, 0);
            }
            return;
        }
    }
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        if (g_menu->unk2D0.lo == g_menu->unk2D0.hi) {
            SaveScreenOpen();
            g_persona_data_step -= 3;
        } else {
            SavePortOpen();
            g_persona_data_step -= 1;
        }
    }
}

void SaveConfirmStep(void)
{
    int kind;
    int ends;

    MenuStepCursor(&g_menu->unk2B0);
    SlotSetPos(3, 0x23, 0xF8, g_menu->unk2B0.cur * 16 + 0xBA);
    if (InputCheckAcceptA(1)) {
        if (g_menu->unk2B0.cur != 0) {
            goto close;
        }
    retry:
        kind = g_save_kinds[g_menu->unk2C0.cur];
        ends = 0;
        if (kind != SAVE_EMPTY) {
            SaveMessage(8, 0);
            if (g_msg_answer) {
                goto close;
            }
            CardDeleteFile(g_menu->unk2D0.cur, g_menu->unk2C0.cur, kind);
        }
        kind = 0;
        switch (g_prev_map) {
        case MAP_SAVE_A:
        case MAP_SAVE_B:
        case MAP_SAVE_C:
            ends = 1;
            kind = 1;
            break;
        }
        func_800AB1EC();
        BgMapInit(g_save_busy_msg, 0);
        MSG_WINDOW(0x30);
        CinemaOpen(1);
        while (!(g_msg->flags & MSG_DONE)) {
            MsgStep();
            RunFrame();
        }
        RunFrame();
        VSync(0);
        if (CardSave(g_menu->unk2D0.cur, g_menu->unk2C0.cur, kind) == 0) {
            SaveScanSlots(g_menu->unk2D0.cur);
            SaveSlotsDraw();
            RunFrame();
            if (ends) {
                SaveMessage(7, 1);
                g_menu_allow_hold = 1;
                g_facility_leave = 1;
            } else {
                SaveMessage(2, 1);
            }
            goto close;
        }
        CinemaClose(1);
        SaveScanSlots(g_menu->unk2D0.cur);
        SaveSlotsDraw();
        VSync(0);
        kind = CardLoad(g_menu->unk2D0.cur);
        switch (kind) {
        case CARD_ERROR:
            VSync(0);
            if (CardLoad(g_menu->unk2D0.cur) != kind) {
                goto retry;
            }
            SaveMessage(3, 0);
            SaveScanSlots(g_menu->unk2D0.cur);
            goto close;
        case CARD_NONE:
            SaveMessage(0, 0);
            SaveScanSlots(g_menu->unk2D0.cur);
            SaveSlotsDraw();
            goto close;
        case CARD_UNFORMAT:
            SaveOfferFormat(0);
            SaveScanSlots(g_menu->unk2D0.cur);
            goto close;
        case CARD_SWAPPED:
            VSync(0);
            SaveScanSlots(g_menu->unk2D0.cur);
            SaveSlotsDraw();
            goto retry;
        default:
            VSync(0);
            SaveScanSlots(g_menu->unk2D0.cur);
            SaveSlotsDraw();
            if (CardCheckFree(g_menu->unk2D0.cur)) {
                SaveMessage(1, 0);
                SaveScanSlots(g_menu->unk2D0.cur);
            }
            goto close;
        }
    }
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
    close:
        SaveSlotsDraw();
        SlotSetFlicker(4, 1);
        func_800AB1EC();
        g_persona_data_step -= 1;
    }
}

/* An unformatted card: asks, and formats it on a yes. 0 if the card is still
   unusable. */
short SaveOfferFormat(short keep)
{
    VSync(0);
    if (CardLoad(g_menu->unk2D0.cur) != CARD_UNFORMAT) {
        return 1;
    }
    MSG_WINDOW(0x30);
    BgMapInit(g_save_format_ask, 0);
    if (!keep) {
        CinemaOpen(1);
    } else {
        g_bg_shown |= 0x10;
    }
    while (!(g_msg->flags & MSG_DONE)) {
        MsgStep();
        RunFrame();
    }
    RunFrame();
    if (!g_msg_answer) {
        BgMapInit(g_save_format_msg, 0);
        while (!(g_msg->flags & MSG_DONE)) {
            MsgStep();
            RunFrame();
        }
        VSync(0);
        if (CardFormat(g_menu->unk2D0.cur)) {
            SaveMessage(5, 1);
            return 1;
        }
        SaveMessage(6, 1);
        return 0;
    }
    CinemaClose(1);
    return 0;
}

void SaveScanSlots(short chan)
{
    int found0;
    int found1;
    int i;
    short empty;

    VSync(0);
    found0 = CardScanSaves(chan, 0, g_save_list[0]);
    VSync(0);
    found1 = CardScanSaves(chan, 1, g_save_list[1]);
    if (found0 != -1 && found1 != -1) {
        for (i = 0; i < SAVE_SLOTS; i++) {
            g_save_kinds[i] = SAVE_EMPTY;
            if ((found0 >> i) & 1) {
                g_save_kinds[i] = 0;
            } else if ((found1 >> i) & 1) {
                g_save_kinds[i] = 1;
            }
        }
        return;
    }
    empty = SAVE_EMPTY;
    for (i = SAVE_SLOTS - 1; i >= 0; i--) {
        g_save_kinds[i] = empty;
    }
}

void SaveSlotsOpen(void)
{
    func_8008EDBC(0x21);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x16, 0x10, MAP_W);
    SaveSlotsDraw();
    SlotInitTagged(g_save_slot_cur_def, 4, 0x42, 0x48,
                   g_menu->unk2C0.cur * 24 + 0x24);
    SlotSetFlicker(4, 1);
    SlotInitTagged(g_fm_hint_def, 0x35, 0x24, 0x40, 0x10);
    SlotInitTagged(g_fm_hint2_def, 0x36, 0x22, 0x40, 0x10);
    SlotSetAnim(0x36, 0, 0, 0, g_menu->unk2D0.cur * 0x30 + 0x60, 0x30, 0, 0);
    func_800AB1EC();
}

/* Two rows a slot: the frame and labels, then the save's name, level and
   time, or blanks for an empty slot. */
void SaveSlotsDraw(void)
{
    int    i;
    int    r;
    short  kind;
    short *name;

    for (i = 0; i < SAVE_SLOTS; i++) {
        TileMapDrawBox(AT(g_tilemap0, i * 2 + 1, 1), 0x14, 2, MAP_W);
        r = i & 0xF;
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, r * 2 + 1, 0), 0x383, 2);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, r * 2 + 1, 7), 0x364, 4);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, r * 2, 16), 0x467, 3);
        *AT(g_tilemap1, r * 2 + 1, 14) = 0xCB;
        kind = g_save_kinds[i];
        if (kind != SAVE_EMPTY) {
            name = AT(g_tilemap1, r * 2, 1);
            TileMapFillRect(name, 0, 8, 1, MAP_W);
            TileMapWriteRow(g_save_list[kind][i].name, name, 0, 8);
            TileMapFillRect(AT(g_tilemap1, r * 2 + 1, 3), 0, 2, 1, MAP_W);
            TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, r * 2 + 1, 4), 0xC0,
                FormatDecimal(g_save_list[kind][i].level, g_hud_digits, 2));
            TileMapFillRect(AT(g_tilemap1, r * 2 + 1, 12), 0, 2, 1, MAP_W);
            TileMapFillRect(AT(g_tilemap1, r * 2 + 1, 15), 0, 2, 1, MAP_W);
            *AT(g_tilemap1, r * 2 + 1, 15) = 0xC0;
            TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, r * 2 + 1, 13), 0xC0,
                FormatDecimal(g_save_list[kind][i].hours, g_hud_digits, 2));
            TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, r * 2 + 1, 16), 0xC0,
                FormatDecimal(g_save_list[kind][i].minutes, g_hud_digits, 2));
            if (kind == 0) {
                TileMapFillRect(AT(g_tilemap1, r * 2, 15), 0x47C, 1, 1, MAP_W);
            } else {
                TileMapFillRect(AT(g_tilemap1, r * 2, 15), 0x47D, 1, 1, MAP_W);
            }
        } else {
            TileMapFillRect(AT(g_tilemap1, r * 2, 1), 0x1A3, 8, 1, MAP_W);
            TileMapFillRect(AT(g_tilemap1, r * 2 + 1, 3), 0x1A3, 2, 1, MAP_W);
            TileMapFillRect(AT(g_tilemap1, r * 2 + 1, 12), 0x1A3, 2, 1, MAP_W);
            TileMapFillRect(AT(g_tilemap1, r * 2 + 1, 15), 0x1A3, 2, 1, MAP_W);
            TileMapFillRect(AT(g_tilemap1, r * 2, 15), 0x1A3, 1, 1, MAP_W);
        }
    }
}

/* One of the screen's messages over the window, waited out. */
void SaveMessage(short n, short keep)
{
    MSG_WINDOW(0x30);
    BgMapInit(g_save_msgs[n], 0);
    if (!keep) {
        CinemaOpen(1);
    } else {
        g_bg_shown |= 0x10;
    }
    while (!(g_msg->flags & MSG_DONE)) {
        MsgStep();
        RunFrame();
    }
    CinemaClose(1);
    RunFrame();
    RunFrame();
}

void SavePortOpen(void)
{
    func_8008EDBC(0x20);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x1C, 6, MAP_W);
    TileMapDrawBox(g_tilemap0 + MAP_W + 1, 0x1A, 4, MAP_W);
    BgMapInit(g_save_port_msg2, 0);
    g_bg_layers[4].x = 0x48;
    g_bg_layers[4].y = 0x2C;
    g_bg_layers[4].w = 0xF0;
    g_bg_layers[4].h = 0x20;
    g_bg_shown |= 0x10;
    SlotInitTagged(g_fm_prompt_cur_def, 3, 0x23, 0xE8,
                   g_menu->unk2E0.cur * 16 + 0x6A);
    SlotInitTagged(g_fm_hint_def, 8, 0x24, 0xE8, 0x68);
    SlotInitTagged(g_fm_hint_def, 9, 0x24, 0xE8, 0x78);
    SlotInitTagged(g_fm_hint2_def, 0xB, 0x22, 0xE8, 0x68);
    SlotInitTagged(g_fm_hint2_def, 0xC, 0x22, 0xE8, 0x78);
    SlotSetAnim(0xB, 0, 0, 0, 0x60, 0x30, 0, 0);
    SlotSetAnim(0xC, 0, 0, 0, 0x90, 0x30, 0, 0);
    SlotSetFlicker(3, 1);
    SlotClear(4);
    SlotClear(5);
    SlotClear(6);
    SlotClear(0x35);
    SlotClear(0x36);
}
