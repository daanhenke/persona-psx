/* Persona 1 (JP) - the persona list panel.
 *
 *   ADV @ 0x80098A40   DNG @ 0x80099208   S2D @ 0x800896B8
 *
 * Ten cells by eleven are cleared, and then nine dashed rules go down two rows
 * apart - eight long-vowel marks each, which is what the font has that reads
 * as a rule. A persona name is drawn over the first few of them, as many as
 * the list holds, so the empty slots keep their rule showing.
 *
 * The ids come out of the same scratch run in the work area that items.c
 * reaches as the pending item list.
 */
#include <types.h>

/* The rules, and the names drawn over them. */
#define LIST_RULES  9
#define RULE_GLYPHS 8

/* The panel, in cells of the layer it is drawn into. */
#define PANEL_W      0xA
#define PANEL_H      0xB
#define LAYER_STRIDE 0x28

/* Two rows between one entry and the next. */
#define ROW_PITCH (LAYER_STRIDE * 2)

/* Where a rule's glyphs are taken from in the font. */
#define RULE_BASE 0xD7

/* Both the layer and the id run are reached by hardcoded address; S2D's sit
   0x20000 higher, which is what WORK_BIAS carries. */
#define g_layer        ((short *)(0x800EF580 + WORK_BIAS))
#define g_persona_list ((u_char *)(0x800EAE4C + WORK_BIAS))

extern u_char g_persona_list_rule[];
extern u_char g_persona_list_count;

extern void TileMapWriteRow(const u_char *src, short *dst, int base,
                            u_short count);
extern void TileMapFillRect(short *dst, short value, u_short w, u_short h,
                            u_short stride);
extern void DrawPersonaName(int id, short *dst, int kind);

void DrawPersonaList(void)
{
    short *cell;
    int    i;

    TileMapFillRect(g_layer, 0, PANEL_W, PANEL_H, LAYER_STRIDE);

    /* The rules start one cell in; the names are drawn flush left. */
    i = 0;
    cell = &g_layer[1];
    do {
        TileMapWriteRow(g_persona_list_rule, cell, RULE_BASE, RULE_GLYPHS);
        i++;
        cell += ROW_PITCH / 2;
    } while (i < LIST_RULES);

    i = 0;
    if (i < g_persona_list_count) {
        cell = g_layer;
        do {
            DrawPersonaName(g_persona_list[i], cell, 0);
            cell += ROW_PITCH / 2;
            i++;
        } while (i < g_persona_list_count);
    }
}
