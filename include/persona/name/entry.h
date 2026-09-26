#ifndef PERSONA_NAME_ENTRY_H
#define PERSONA_NAME_ENTRY_H

/* Persona 1 (JP) - the name-entry screen's state.
 *
 * Three fields are entered in turn. Each has room for eight cells, but only
 * the third is scanned that far: the first two stop at five. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#define NAME_FIELDS 3
#define NAME_CELLS  8
#define NAME_SHORT  5   /* how far the first two fields are used */

/* The keyboard: four pages of six rows, each row ten keys with a button at
   either end (columns 0 and 11). */
#define KEY_PAGES 4
#define KEY_ROWS  6
#define KEY_COLS  10

/* Pad bits. */
#define PAD_SELECT 0x0100
#define PAD_START  0x0800
#define PAD_UP     0x1000
#define PAD_RIGHT  0x2000
#define PAD_DOWN   0x4000
#define PAD_LEFT   0x8000
#define PAD_L2     0x0001
#define PAD_R2     0x0002
#define PAD_L1     0x0004
#define PAD_R1     0x0008
#define PAD_TRI    0x0010
#define PAD_CIRCLE 0x0020
#define PAD_CROSS  0x0040
#define PAD_SQUARE 0x0080


/* One sprite as the screen sets it up. */
typedef struct {
    /* 0x00 */ int     x;
    /* 0x04 */ int     y;
    /* 0x08 */ u_char  w, h;
    /* 0x0A */ u_char  tpage;
    /* 0x0B */ u_char  u, v;
    /* 0x0D */ u_char  pad0D;
    /* 0x0E */ short   cx, cy;
    /* 0x12 */ u_char  attr;
    /* 0x13 */ u_char  pad13;
} NameSpriteDef;

#define NAME_SPRITES 50

/* The kanji list: one group per kana, each a run of rows of ten; the list
   keeps seven rows in a ring and shows six. */
#define KANJI_GROUPS 44
#define LIST_SLOTS   7
#define LIST_ROWS    6

/* The entry state: separate variables, which is why the arrays each start
   on a word. */
extern u_char      g_name_repeat;      /* frames the pad has been held */
extern u_char      g_name_first;       /* still in the first, longer delay */
extern u_char      g_name_mode;        /* 0 keyboard, 1 kanji list */
extern u_char      g_name_field;       /* which field is being edited */
extern u_char      g_name_page;        /* keyboard page */
extern u_short     g_name_text[NAME_FIELDS][NAME_CELLS];
extern signed char g_name_cursor[NAME_FIELDS]; /* cell the caret sits on */
extern short       g_name_col;         /* key under the cursor */
extern short       g_name_row;
extern u_char      g_name_blink;       /* caret brightness */
extern u_char      g_name_blink_step;
extern u_char      g_name_x[NAME_FIELDS]; /* where each field draws */
extern u_char      g_name_y[NAME_FIELDS];
extern u_char      g_name_len[NAME_FIELDS]; /* cells each field takes */
extern u_short     g_name_code_base[2];    /* first code of each kana page */
extern NameSpriteDef g_name_sprite_defs[NAME_SPRITES];
extern u_short       g_keyboard[KEY_PAGES][KEY_ROWS][KEY_COLS];
extern u_char        g_kanji_row_counts[]; /* rows in each group */
extern u_short       g_kanji[][KEY_COLS];
extern u_short     (*g_kanji_rows[])[KEY_COLS]; /* each group's rows of ten */
extern int           g_name_labels[];

extern u_long   g_pad_prev;
extern u_long   g_pad_trig;
extern short    g_name_list_pos;
extern u_long   g_ot_tags[2][16];
extern GsBG     g_bg_frame;
extern GsBG     g_bg_fields;
extern PACKET   g_packets[2][0x8000];
extern GsOT     g_ot[2];
extern u_long   g_sprite_attr[NAME_SPRITES];
extern GsBG     g_bg_keys;
extern GsMAP    g_map_frame;
extern GsMAP    g_map_fields;
extern u_char   g_list_rows[][2];
extern GsBG     g_bg_list;
extern u_char   g_blink_tick;
extern u_long   g_glyph_cell[];
extern GsMAP    g_map_keys;
extern GsSPRITE g_sprites[NAME_SPRITES];
extern short    g_list_top;
extern GsMAP    g_map_list;
extern u_long   g_pad_held;
extern u_long   g_pad_raw;
extern short    g_list_code;
extern u_short  g_caret_x;
extern u_short  g_caret_y;

/* The load buffer: a table of offsets to each file the loader read. */
#define NAME_PACK      ((u_long *)0x80140000)
#define g_seq_handles  ((short *)0x801F537C)
#define g_vab_id       (*(short *)0x801F535C)

/* Sprites the screen moves or hides by number. */
#define SPR_PAGE_TAB   7    /* 7..10: one tab per keyboard page */
#define SPR_KEY_CARET  26   /* the box round a key */
#define SPR_SIDE_CARET 27   /* the box round an end button */
#define SPR_TEXT_CARET 28   /* under the cell being typed into */
#define SPR_PAGE_TAB2  29   /* 29..32 */
#define SPR_PREVIEW    49

#define ATTR_HIDE 0x80

void ExpandGlyph(u_short code, u_long *dst, int stride);
void UploadImage(int x, int y, int w, int h, u_long *data);
void NameDrawKeyboardPage(int page);
void NameDrawCursor(void);
void NamePadRead(void);
void NamePlaySe(int se);
void NameSpriteSet(u_short i, u_short w, u_short h, u_short tpage, u_short u,
                   u_short v, short cx, short cy, int attr);
void NameSpriteFromDef(u_char i);

#endif
