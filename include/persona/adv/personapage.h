#ifndef PERSONA_ADV_PERSONAPAGE_H
#define PERSONA_ADV_PERSONAPAGE_H

/* Persona 1 (JP) - the Persona page ADV draws in two places: the persona
 * data screen (personadata.c) and the status menu's Persona view
 * (statuspersona.c).
 *
 * Both read the Persona's portrait off the disc into a staging buffer and
 * queue it into VRAM, lay the page out in the two character-map layers and
 * scroll between its two halves, 8 lines a frame.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/slot.h>
#include <persona/common/tilemap.h>

#define g_slots    ((Slot *)0x800DC10C)
#define g_tilemap0 ((short *)0x800EE180)

/* A cell of a character-map layer, by row and column. */
#define AT(map, row, col) (&(map)[(row) * MAP_W + (col)])

/* Where a portrait is read to: the archive entry's eight-byte header and
   then the TIM. */
#define PORTRAIT_READ ((u_long *)0x800F4000)
#define PORTRAIT_TIM  ((u_long *)0x800F4008)

/* The sprites. */
#define PICK_CURSOR_SLOT 2
#define ARROW_L_SLOT     0x22
#define ARROW_R_SLOT     0x23
#define PAGE_TOP_SLOT    0x1E
#define PAGE_BOTTOM_SLOT 0x1F
#define PAGE_MARK_SLOT   32
#define HINT_SLOT        0x2C

/* The page scrolls 8 lines a frame to one of two stops. */
#define PAGE_STEP 8
#define PAGE_LOW  0xE0

#define GLYPH_SLASH 0xCA

/* The two page marks hide at the end they point past. */
#define PAGE_MARKS()                                                          \
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];                                    \
    if (g_cam_y == 0) {                                                       \
        g_slot_cur->attr |= SLOT_ATTR_HIDE;                                   \
    } else {                                                                  \
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                  \
    }                                                                         \
    g_slot_cur++;                                                             \
    if (g_cam_y == PAGE_LOW) {                                                \
        g_slot_cur->attr |= SLOT_ATTR_HIDE;                                   \
    } else {                                                                  \
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                  \
    }

extern void TileMapDrawWindow(short *dst, u_char w, u_char h, u_char stride);
extern void TileMapDrawBox(short *dst, u_short w, short h, u_short stride);
extern void TileMapBlitRle(const u_short *src, short *dst, u_short stride);
extern void TileMapWriteBar(short *dst, u_char width);
extern void func_8008EDBC(int);
extern void DrawSpellName(short spell, short *dst, u_short base, short rule);
extern void CellsClear(GsCELL *dst, u_char count);
extern void CellsWriteRow(GsCELL *dst, const u_char *src, u_char page,
                          u_short count);

extern u_char InputCheckAcceptA(u_char repeat);
extern u_char InputCheckAcceptB(u_char repeat);
extern int    MsgStep(void);
extern void   RunFrame(void);
extern void   TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void   AdvResolveSceneLoc(short kind, int index, void *unused);
extern void   DrawPersonaDataStatBars(short id);

extern CdlFILE g_adv_scene_file;
extern short   g_cam_y;
extern short   g_map_scroll_y;
extern Slot   *g_slot_cur;

/* The page's sprites, cell rows and fixed text. */
extern u_char  g_pdata_arrow_l_def[];
extern u_char  g_pdata_arrow_r_def[];
extern u_char  g_pdata_cursor_def[];
extern u_char  g_pdata_top_def[];
extern u_char  g_pdata_bottom_def[];
extern u_char  g_pdata_mark_up_def[];
extern u_char  g_pdata_mark_down_def[];
extern u_short g_pdata_page_rle[];
extern GsCELL  g_pdata_name_cells[];
extern GsCELL  g_pdata_arcana_cells[];
extern u_char  g_resist_labels[];
extern u_char  D_800B92A0[];
extern u_char  D_800B1898[];
extern u_char  D_800B1A98[];
extern u_char  D_800B1E98[];

#endif
