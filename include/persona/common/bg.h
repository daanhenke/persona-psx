#ifndef PERSONA_COMMON_BG_H
#define PERSONA_COMMON_BG_H

/* Persona 1 (JP) - the background layers.
 *
 * Six GsBG layers the draw pass hands to GsSortFastBg, each shown while its
 * bit in g_bg_shown is set. Every overlay that draws a field carries its own
 * copy of the set; the maps they draw live in main (g_bg_maps).
 *
 * The first layer's colour doubles as the screen's brightness: the fades step
 * g_bg_layers[0].r and shade everything else by it.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menulist.h>

#define BG_LAYERS 6

extern GsBG    g_bg_layers[BG_LAYERS];
extern u_short g_bg_layer_otz[BG_LAYERS];
extern u_long  g_bg_shown;
extern u_int  *g_bg_maps[];

/* The character maps of the menu screens' three text layers (0, 1 and 2). */
extern GsMAP   g_bg_map0, g_bg_map1, g_bg_map2;

/* The tiled map layer 4 draws: 16x16-pixel cells, BG_MAP_W across and
   BG_MAP_H down. g_bg_index says which cell goes where. */
#define BG_MAP_W    15
#define BG_MAP_H    4
#define BG_MAP_CELL 16

extern GsMAP   g_bg_map;
extern u_short g_bg_index[BG_MAP_W * BG_MAP_H];

/* The map's cell definitions and the message window's state, both in the work
   area and reached by hardcoded address, so S2D's come out 0x20000 higher on
   the same WORK_BIAS.

   The map is the message window: BgMapInit opens a message by handing it the
   script, and the interpreter (0x80066970) walks the script a glyph at a time
   into the map's cells. */
typedef struct {
    /* 0x00 */ u_int    flags;    /* MSG_*; bits 4-6 also pick the palette
                                     BgMapSetCell gives each glyph, and
                                     BgPanelSet parks a closed window at
                                     MSG_DONE                             */
    /* 0x04 */ u_short  wait;     /* frames left in a scripted pause      */
    /* 0x06 */ u_short  delay;    /* frames left before the next glyph    */
    /* 0x08 */ u_short  speed;    /* what `delay` is put back to          */
    /* 0x0A */ u_short  cursor;   /* the next cell, four rows of fifteen  */
    /* 0x0C */ u_char  *script;   /* where the message is being read      */
    /* 0x10 */ u_char  *sub;      /* where an inserted string is read,
                                     while MSG_SUB is set                 */
    /* 0x14 */ MenuList choice[2];/* a choice's rows, then columns        */
    /* 0x34 */ u_char   choices;
    /* 0x35 */ u_char   left;     /* glyphs left of an inserted string    */
    /* 0x36 */ u_char   digits[8];/* a number being printed               */
    /* 0x3E */ u_char   pad3E[2];
} MsgState;

#define MSG_SUB       0x000001  /* reading `sub`, not `script`             */
#define MSG_SCROLL    0x000002  /* scroll a row before the next glyph      */
#define MSG_FULL      0x000004  /* the window has filled once              */
#define MSG_KEY       0x000008  /* waiting for a key                       */
#define MSG_COUNTED   0x004000  /* the insert is `left` glyphs long        */
#define MSG_DONE      0x008000  /* the message has ended                   */
#define MSG_CHOOSING  0x010000  /* a choice follows the insert             */
#define MSG_CHOICE    0x020000  /* a choice is open                        */
#define MSG_FIRST     0x040000  /* a first name; the surname follows       */
#define MSG_SURNAME   0x080000  /* print the surname next                  */
#define MSG_NEWLINE   0x100000  /* the row was ended by a newline          */
#define MSG_BACKWARD  0x200000  /* read the insert backwards (digits)      */

#define g_bg_cells ((GsCELL *)(0x800E224C + WORK_BIAS))
#define g_msg ((MsgState *)(0x800E1E4C + WORK_BIAS))

extern void BgMapInit(void *script, short speed);
extern void BgMapClearRow(u_short row);
/* The glyph is handed over too, though only the cell's own index is used. */
extern void BgMapSetCell(u_short idx, u_short glyph);

#endif
