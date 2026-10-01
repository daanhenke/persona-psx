/* Persona 1 (JP) - CASINO's entry point and the state it carries.
 *   0x80065DAC ovl_casino_entry
 *   0x80065EA4 CasinoExit
 *   0x80065F10 CasinoRunGame
 *   0x80065FC8 CasinoInitState
 *   0x80066118 CasinoMapWork
 *   0x800665AC CasinoSyncCounters
 *   0x800666C8 CasinoPickGame
 *   0x80066784 CasinoFindCoinItem
 *   0x80066814 CasinoItemSlot
 *   0x800668BC CasinoReadPad
 *
 * The entry clears the work area, sets up, then runs a frame at a time:
 * the pad, the game picked from the map, the draw. The game leaves by
 * setting g_casino_game to CASINO_GAME_DONE, and the exit writes the money
 * and the two counters back into the save.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsnd.h>
#include <libetc.h>
#include <memory.h>
#include <persona/main/state.h>
#include <persona/casino/casino.h>

extern int    rand(void);
extern void   srand(unsigned int seed);

extern u_char D_800A878C;
extern u_char D_800A87D4;
extern u_char D_800A8880;
extern int    D_800A8EA4;
extern u_char D_800A9540;
extern u_int  D_800AAF18;
extern u_char D_800AAF68;
extern short  D_800B0158;
extern int    D_800B0A9C;
extern u_char D_800B0AF0;
extern u_char D_800B0AF8;
extern u_char D_800B0AFC;
extern u_char D_800B0AFD;
extern u_int  D_800B0BB8;
extern u_char D_800B3E98;

/* Load buffers in the work area, per image table, and their counters. */
extern u_char *D_800A837C[8];
extern short   D_800A839C[2];
extern u_char *D_800A86A0[4];
extern short   D_800A86B0[2];
extern u_char *D_800A9798[2];
extern short   D_800A97A0;
extern u_char *D_800AF5BC[3];
extern short   D_800AF5C8;
extern u_char *D_800AF688[3];
extern short   D_800AF694;
extern u_char *D_800B015C[4];
extern u_char *D_800B0974[10];
extern short   D_800B099C[2];
extern short   D_800B0A48[2];
extern u_char *D_800B0AE8;
extern u_char *D_800B0AEC;
extern u_char *D_800B41A4[3];
extern short   D_800B41B0;
extern u_char *D_800B4764[4];
extern short   D_800B4774[2];

extern void CasinoExit(void);
extern void CasinoRunGame(void);
extern void CasinoInitState(void);
extern void CasinoMapWork(void);
extern void CasinoSyncCounters(void);
extern void CasinoPickGame(void);
extern void CasinoFindCoinItem(void);
extern u_short CasinoItemSlot(short id);
extern void CasinoReadPad(void);

extern void CasinoDebugInit(void);
extern void CasinoInitDraw(void);
extern void func_8006C890(void);
extern void CasinoFrameWrap(int *frame);
extern void CasinoDrawFrame(void);
extern void CasinoPlayTimeTick(u_char *clock);
extern void CasinoStopSeqs(void);
extern void func_8006D25C(void);
extern void func_80078294(void);
extern void func_800828DC(void);
extern void func_800886F0(void);
extern void func_8008D250(void);

/* The coin item: its id, and the 9/7 split every bag slot has. */
#define ITEM_COINS 0x23
#define ITEM_COUNT 0x17F
#define ITEM_ID    0x1FF

void ovl_casino_entry(void)
{
    bzero((u_char *)0x800C0000, 0x40000);
    CasinoMapWork();
    CasinoPickGame();
    CasinoInitState();
    CasinoDebugInit();
    CasinoInitDraw();
    func_8006C890();
    SetDispMask(1);
    for (;;) {
        CasinoReadPad();
        CasinoRunGame();
        CasinoFrameWrap(&g_casino_frame);
        CasinoDrawFrame();
        if (g_casino_game == CASINO_GAME_DONE) {
            break;
        }
        if (g_casino_timer == 0x7FFFFFFE) {
            g_casino_timer = 0x7FFFFFFF;
        }
        if (g_casino_clock_on) {
            CasinoPlayTimeTick(g_playtime_hours);
        }
    }
    CasinoExit();
}

/* Puts the money and the two counters back in the save and hands control
   back to the field. */
void CasinoExit(void)
{
    CasinoStopSeqs();
    SsVabClose(g_casino_vab);
    g_state_next = 3;
    g_money2 = g_casino_money;
    g_29B0 = D_800AAF18;
    g_29B4 = D_800B0BB8;
}

void CasinoRunGame(void)
{
    switch (g_casino_game) {
    case 1:
        func_8006D25C();
        break;
    case 2:
        func_80078294();
        break;
    case 3:
        func_800828DC();
        break;
    case 4:
        func_800886F0();
        break;
    case 5:
        func_8008D250();
        break;
    case CASINO_GAME_LEAVE:
        if (g_casino_timer > 0x48) {
            g_casino_game = CASINO_GAME_DONE;
        }
        break;
    }
}

/* Seeds rand from the play clock and clears the per-visit state. The play
   clock only runs if it has not already stopped at 99:59. */
void CasinoInitState(void)
{
    u_char *hours;

    hours = g_playtime_hours;
    g_casino_frame = 0;
    g_casino_timer = 0;
    g_casino_seed = g_playtime_frame + g_playtime_sec * 60;
    srand(g_casino_seed);
    if (g_playtime_min == 59 && *hours == 99) {
        g_casino_clock_on = 0;
    } else {
        g_casino_clock_on = 1;
    }
    D_800A8EA4 = 0xFFFF;
    D_800A87D4 = 0xFF;
    D_800B3E98 = 0;
    D_800A9540 = 0;
    D_800B0A9C = 0;
    D_800A8880 = 0;
    D_800AFC98.b1 = 0;
    D_800AFC98.b0 = 0;
    D_800AFC98.b3 = 0;
    D_800AFC98.b2 = 0;
    D_800AFC98.b5 = 0;
    D_800AFC98.b4 = 0;
    g_casino_money = g_money2;
    CasinoFindCoinItem();
    CasinoSyncCounters();
    D_800B0AFD = 0;
    D_800B0AFC = 0;
    D_800B0AF8 = 0;
}

/* Carves the work area at 0x800C0000: the two draw buffers and their
   ordering tables, then the buffers each table of images loads into. */
void CasinoMapWork(void)
{
    g_casino_db[0].draw = (DRAWENV *)0x800C0000;
    g_casino_db[1].draw = (DRAWENV *)0x800C005C;
    g_casino_db[0].disp = (DISPENV *)0x800C00B8;
    g_casino_db[1].disp = (DISPENV *)0x800C00CC;
    g_casino_db[0].ot = (u_long *)0x800C00E0;
    g_casino_db[1].ot = (u_long *)0x800C10E0;
    ClearOTag(g_casino_db[0].ot, 0x400);
    ClearOTag(g_casino_db[1].ot, 0x400);
    D_800B015C[0] = (u_char *)0x800C20E0;
    D_800B015C[1] = (u_char *)0x800CBD20;
    D_800B015C[2] = (u_char *)0x800D5960;
    D_800B015C[3] = (u_char *)0x800D6130;
    D_800A9798[0] = (u_char *)0x800D6518;
    D_800A9798[1] = (u_char *)0x800D6618;
    D_800A97A0 = 0;
    D_800B4764[0] = (u_char *)0x800D6698;
    D_800B4764[1] = (u_char *)0x800DA518;
    D_800B4764[2] = (u_char *)0x800DE398;
    D_800B4764[3] = (u_char *)0x800DEB68;
    D_800B4774[0] = 0;
    D_800B4774[1] = 0;
    D_800A837C[0] = (u_char *)0x800DF338;
    D_800A837C[1] = (u_char *)0x800E1278;
    D_800A837C[2] = (u_char *)0x800E31B8;
    D_800A837C[3] = (u_char *)0x800E3988;
    D_800A837C[4] = (u_char *)0x800E4158;
    D_800A837C[5] = (u_char *)0x800E4928;
    D_800A837C[6] = (u_char *)0x800E50F8;
    D_800A837C[7] = (u_char *)0x800E58C8;
    D_800A839C[0] = 0;
    D_800A839C[1] = 0;
    D_800B0974[0] = (u_char *)0x800E6098;
    D_800B0974[1] = (u_char *)0x800E7FD8;
    D_800B0974[2] = (u_char *)0x800E9F18;
    D_800B0974[3] = (u_char *)0x800EBE58;
    D_800B0974[4] = (u_char *)0x800EDD98;
    D_800B0974[5] = (u_char *)0x800EE568;
    D_800B0974[6] = (u_char *)0x800EED38;
    D_800B0974[7] = (u_char *)0x800EF508;
    D_800B0974[8] = (u_char *)0x800EFCD8;
    D_800B0974[9] = (u_char *)0x800F04A8;
    D_800B099C[0] = 0;
    D_800B099C[1] = 0;
    D_800A86A0[0] = (u_char *)0x800F0C78;
    D_800A86A0[1] = (u_char *)0x800F0D78;
    D_800A86A0[2] = (u_char *)0x800F0E78;
    D_800A86A0[3] = (u_char *)0x800F0F78;
    D_800A86B0[0] = 0;
    D_800A86B0[1] = 0;
    D_800B0AEC = (u_char *)0x800F1078;
    D_800B0AE8 = (u_char *)0x800F3F58;
    D_800AF5BC[0] = (u_char *)0x800F6E38;
    D_800AF5BC[1] = (u_char *)0x800F7038;
    D_800AF5BC[2] = (u_char *)0x800F7138;
    D_800AF5C8 = 0;
    D_800AF688[0] = (u_char *)0x800F71B8;
    D_800AF688[1] = (u_char *)0x800F73B8;
    D_800AF688[2] = (u_char *)0x800F74B8;
    D_800AF694 = 0;
    D_800B41A4[0] = (u_char *)0x800F7538;
    D_800B41A4[1] = (u_char *)0x800F7A38;
    D_800B41A4[2] = (u_char *)0x800F7AB8;
    D_800B41B0 = 0;
    D_800B0A48[0] = 0;
    D_800B0A48[1] = 0;
}

/* With the flag in the save set, whatever the running total gained since
   the last visit is added to both counters. The copies the casino shows
   are capped at eight and six digits. */
void CasinoSyncCounters(void)
{
    int d;

    D_800AAF68 = D_801F29ED;
    D_800AAF68 = (D_800AAF68 >> 4) & 1;
    if (D_800AAF68) {
        d = D_801F1BDC - g_exp_carry;
        if (d > 0) {
            g_29B0 += d;
        }
        d = D_801F1BDC - g_exp_carry;
        if (d > 0) {
            g_29B4 += d;
        }
        g_exp_carry = D_801F1BDC;
    }
    D_800AAF18 = g_29B0;
    D_800B0BB8 = g_29B4;
    if (D_800AAF18 > 99999998) {
        D_800AAF18 = 99999999;
    }
    if (D_800B0BB8 > 999998) {
        D_800B0BB8 = 999999;
    }
}

/* The map leaves the spot the player stood at; each range of spots is one
   game. */
void CasinoPickGame(void)
{
    g_casino_spot = g_script_534C;
    g_casino_step = 0x11;
    switch (g_casino_spot) {
    case 0xD0: case 0xD1: case 0xD2: case 0xD3:
        g_casino_game = 1;
        D_800B0158 = 0x2F6;
        break;
    case 0xD4: case 0xD5: case 0xD6: case 0xD7:
        g_casino_game = 2;
        D_800B0158 = 0x3C1;
        break;
    case 0xD8: case 0xD9: case 0xDA: case 0xDB:
    case 0xDC: case 0xDD: case 0xDE: case 0xDF:
    case 0xE0: case 0xE1: case 0xE2: case 0xE3:
    case 0xE4: case 0xE5: case 0xE6: case 0xE7:
        g_casino_game = 3;
        D_800B0158 = 0x22E;
        break;
    case 0xE9: case 0xEA: case 0xEB: case 0xEC:
    case 0xED: case 0xEE: case 0xEF: case 0xF0:
    case 0xF1: case 0xF2: case 0xF3: case 0xF4:
    case 0xF5: case 0xF6: case 0xF7: case 0xF8:
    case 0xF9: case 0xFA: case 0xFB: case 0xFC:
    case 0xFD: case 0xFE: case 0xFF:
        g_casino_game = 4;
        D_800B0158 = 0x14D;
        break;
    case 0xE8:
        g_casino_game = 5;
        D_800B0158 = 0x1A0;
        break;
    }
}

/* Finds the coin item in the bag, or makes an empty slot into it. */
void CasinoFindCoinItem(void)
{
    short   i;
    u_short v;

    i = CasinoItemSlot(ITEM_COINS);
    g_casino_coin_item = &g_items[i];
    v = *g_casino_coin_item;
    if ((v & ITEM_ID) == ITEM_COINS) {
        D_800A878C = v >> 9;
    } else {
        D_800B0AF0 = 0;
        D_800A878C = 0;
        *g_casino_coin_item = 0;
        *g_casino_coin_item |= ITEM_COINS;
    }
}

/* The slot holding item `id`; failing that the first slot with nothing
   counted in it, then the first with no id; 0xFFFF if the bag is full. */
u_short CasinoItemSlot(short id)
{
    u_short *p;
    int      i;
    int      free_at;
    int      blank_at;
    u_char   n_free;
    u_char   n_blank;

    free_at = 0;
    blank_at = 0;
    n_free = 0;
    n_blank = 0;
    for (i = 0; i < ITEM_COUNT; i++) {
        u_short v;
        int     count;

        v = g_items[i];
        count = v >> 9;
        if ((v & ITEM_ID) == id) {
            return i;
        }
        if ((v & ITEM_ID) == 0 && n_blank == 0) {
            blank_at = i;
            n_blank++;
        }
        if (count == 0 && n_free == 0) {
            free_at = i;
            n_free++;
        }
    }
    if (n_free) {
        return free_at;
    }
    if (n_blank) {
        return blank_at;
    }
    return 0xFFFF;
}

/* The buttons held, and those newly down this frame. */
void CasinoReadPad(void)
{
    u_long prev;

    prev = g_casino_pad;
    g_casino_pad = PadRead(1);
    g_casino_pad_trig = (prev & g_casino_pad) ^ g_casino_pad;
}
