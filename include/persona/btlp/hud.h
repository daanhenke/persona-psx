#ifndef PERSONA_BTLP_HUD_H
#define PERSONA_BTLP_HUD_H

/* Persona 1 (JP) - the status panel along the foot of the battle screen.
 *
 * One record carries the whole panel: where it is drawn, the matrix it is put
 * up through, the draw modes its two passes take and the six pieces its frame
 * is made of. hud.c ramps the scales, hudload.c puts the record back to its
 * resting place and BtlHudDraw builds the primitives.
 *
 * The panel is drawn through the GTE, so the pieces are corners rather than
 * sprites whenever either scale is short of full size; at full size the draw
 * is the cheaper sprite pass.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

typedef struct {
    /* 0x00 */ short   x;          /* where the panel sits on the screen, which
                                      is also the geometry offset it is drawn
                                      through                                */
    /* 0x02 */ short   y;
    /* 0x04 */ VECTOR  trans;      /* the matrix the panel is put up with     */
    /* 0x14 */ SVECTOR rot;
    /* 0x1C */ VECTOR  scale;      /* reached by address for ScaleMatrix; the
                                      three below are the same words by name */
    /* 0x2C */ u_char  pad2C[0x48];
    /* 0x74 */ DR_MODE mode[2][2]; /* one per pass, one per ordering table    */
    /* 0xA4 */ u_char  padA4[0x30];
    /* 0xD4 */ short   piece_xy[6][2]; /* the six pieces of the frame: where
                                          each is put, which cell of the sheet
                                          it is and how big it is            */
    /* 0xEC */ u_char  piece_uv[6][4];
    /* 0x104 */ u_short piece_wh[6][2];
} BtlHud;                          /* 0x11C bytes */

extern BtlHud g_btl_hud;

/* The zoom's three scales. Every unit that reads or writes a scale does so
   by name and not through the record, which is what keeps each access on its
   own address rather than off a shared base. Both ramp to BTL_HUD_FULL. */
extern int g_btl_hud_scale;
extern int g_btl_hud_scale_y;
extern int g_btl_hud_scale_z;

/* Which cell of the sheet each of the ninety squares of the panel's face is
   drawn from, 0xFF for a square that is left empty. The low nibble is the
   column and the high nibble the row, and a cell from row five on is half a
   cell further down the sheet. */
extern u_char g_btl_hud_cells[];

#define BTL_HUD_CELLS 0x5A
#define BTL_HUD_EMPTY 0xFF

/* Whether the panel is drawn at all, which phase of the zoom it is in, and how
   long it has left to sit at full size. */
extern short  g_btl_hud_flags;
extern u_char g_btl_hud_state;
extern u_char g_btl_hud_hold;

#define BTL_HUD_DRAWN 0x8000

/* Full size, in the twelve-fraction-bit fixed point the rest of the overlay
   scales with. */
#define BTL_HUD_FULL 0x1000

extern void BtlHudShow(void);
extern void BtlHudHide(void);
extern int  BtlHudState(void);
extern void BtlHudTick(void);
extern void BtlHudDraw(void);

#endif
