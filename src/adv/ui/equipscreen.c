/* Persona 1 (JP) - the equipment screen.  ADV only.
 *   0x80091608 EquipScreen
 *
 * The screen and its loop; the four steps it runs, by g_equip_step, follow
 * it in the image (still asm). Opened from the status menu it shares that
 * menu's lists and fades; `standalone` is script command 4C's way in, after
 * FormationMenu has set the menu context up, and makes it set up its own
 * lists, open its sounds and fade itself in and out.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/adv/personapage.h>

#define g_seq_handle ((short *)0x801F537C)

#define STEP_DONE 0xFF
#define PAD_TOGGLE 0x100

extern u_char  g_equip_step;
extern int     g_BB998;
extern u_char  g_BC5C8;
extern u_char  g_menu_allow_hold;
extern u_short g_key_menu_close;
extern int     g_pad_pressed[];
extern u_char  g_party_last;
extern u_char  D_800B1D08[];
extern u_char  D_800B2330[];
extern u_char  D_800B12A8[];
extern u_char  g_fm_prompt_cur_def[];

extern u_char PartyLastSlot(void);
extern void   DrawStatusFrames(void);
extern void   DrawGauge(int);
extern void   SoundOpenSeq(u_short slot, u_short seq, short vab);
extern void   SoundPlaySeq(u_short slot, u_short seq, short vab);
extern void   FadeUpBlocking(short step, short limit);
extern void   FadeDownBlocking(short step, short floor);
extern void   SsSetNck(short handle);
extern void   func_800936D4(short member);
extern void   func_8009320C(void);
extern void   func_80091C44(void);
extern void   func_8009240C(void);
extern void   func_80092B9C(void);
extern void   func_80092ED4(void);

void EquipScreen(short standalone)
{
    int i;

    if (standalone) {
        g_party_last = PartyLastSlot();
        MenuListInit(&g_menu->unk050, 0, 0, g_party_last, 0x1A);
        MenuListInit(&g_menu->unk220, 0, 0, 8, 0x16);
        MenuListInit(&g_menu->unk230, 0, 0, 3, 0x14);
        MenuListInit(&g_menu->unk240, 0, 0, 1, 0x1A);
        MenuListInit(&g_menu->unk250, 0, 0, 1, 0x1A);
    }
    func_8008EDBC(3);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    DrawStatusFrames();
    TileMapWriteRow(D_800B92A0 + 0x4C, AT(g_tilemap1, 3, 19), 0, 3);
    TileMapWriteRow(D_800B92A0 + 0x4F, AT(g_tilemap1, 4, 19), 0, 3);
    TileMapWriteRow(D_800B92A0 + 0x52, AT(g_tilemap1, 5, 19), 0, 3);
    TileMapWriteRow(D_800B92A0 + 0x55, AT(g_tilemap1, 6, 19), 0, 3);
    TileMapWriteRow(D_800B92A0 + 0x58, AT(g_tilemap1, 7, 19), 0, 3);
    DrawGauge(0);
    for (i = 0; i < 7; i++) {
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 2 + i, 3),
                        (u_short)(0x3B1 + i * 2), 2);
    }
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 2, 27), 0x3B1, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 3, 27), 0x3B1, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 4, 27), 0x3B3, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 5, 27), 0x3B3, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 2, 29), 0x3BF, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 3, 29), 0x3C2, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 4, 29), 0x3BF, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 5, 29), 0x3C2, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 6, 28), 0x3C5, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 7, 28), 0x3C8, 3);
    func_800936D4(g_menu->unk050.cur);
    SlotClearAll();
    SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x90, 0, 0, 0);
    SlotInitTagged(g_pdata_cursor_def, 1, 0x42, 0x20, 0x78);
    SlotInitTagged(g_fm_prompt_cur_def, 2, 0x42, 0x18, 0x84);
    SlotInitTagged(g_pdata_cursor_def, 3, 0x42, 0x38, 0x78);
    SlotInitTagged(D_800B12A8, 5, 0x42, 0x18, 0x84);
    SlotSetFlicker(1, 1);
    SlotSetFlicker(2, 1);
    SlotSetFlicker(3, 1);
    SlotSetFlicker(5, 1);
    func_8009320C();
    g_equip_step = 0;
    if (standalone) {
        SoundOpenSeq(0x18, 0, 0);
        SoundOpenSeq(0x19, 0, 0);
        SoundOpenSeq(0x1A, 0, 0);
        SoundOpenSeq(0x1B, 0, 0);
        SetDispMask(1);
        FadeUpBlocking(8, 0x80);
    }
    while (g_equip_step != STEP_DONE) {
        RunFrame();
        switch (g_equip_step) {
        case 0:
            func_80091C44();
            break;
        case 1:
            func_8009240C();
            break;
        case 2:
            func_80092B9C();
            break;
        case 3:
            func_80092ED4();
            break;
        }
        if (g_pad_pressed[0] & PAD_TOGGLE) {
            g_BB998 ^= 1;
        }
        if (!g_menu_allow_hold && (g_key_menu_close & g_pad_pressed[0])) {
            g_menu_allow_hold = 1;
            SoundPlaySeq(0x18, 0, 1);
        }
    }
    if (standalone) {
        FadeDownBlocking(8, 0);
        g_BC5C8 = 0;
        SsSetNck(g_seq_handle[0x18]);
        SsSetNck(g_seq_handle[0x19]);
        SsSetNck(g_seq_handle[0x1A]);
        SsSetNck(g_seq_handle[0x1B]);
    }
}
