/* Persona 1 (JP) - an item's stats as a line of text.  ADV only.
 *   0x8007D5DC TextItemStatRow
 *
 * The shop and the item screens show one line about the item under the
 * cursor, as a background layer of its own - the same layer and cell buffer
 * the automap's name banner uses. Most items are described by a string of
 * their own; equipment instead has its numbers spelled out after labels:
 * weapons and guns their power, rate and a third label picked by the record,
 * the next run only its power, and the rest their power and rate under
 * another pair of labels. Item 0 takes the line down.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/bg.h>
#include <persona/common/item.h>

/* Where each run of items starts. Weapons and guns read the same, and gcc
   folds the two branches back into one - the test between them is gone from
   the image. */
#define ITEM_WEAPONS 0xA3
#define ITEM_GUNS    0x103
#define ITEM_ARMOUR  0x133
#define ITEM_OTHER   0x13F

/* The layer the line is drawn on, and its size. */
#define LINE_LAYER 4
#define LINE_BIT   0x10
#define LINE_W     0xF0
#define LINE_H     0x10

/* The labels the numbers follow. The third a weapon shows is picked by its
   `hits`, which counts from 15. */
#define LABEL_POWER  0
#define LABEL_RATE   1
#define LABEL_POWER2 2
#define LABEL_RATE2  3
#define LABEL_HITS0  15
extern u_char *g_stat_labels[];

extern u_char *g_item_descs[];
extern u_char  g_map_name_cells[];

extern void     bzero(void *dst, int len);
extern u_char *TextCopyUntilEnd(u_char *dst, u_char *src);
extern u_char *TextAppendNumber(u_char *dst, short value, u_char width);

void TextItemStatRow(short item, short x, short y)
{
    MsgState *m;
    u_char   *p;

    p = g_map_name_cells;
    m = g_msg;
    bzero(p, 0x22);
    if (item == 0) {
        goto clear;
    }

    if (item >= ITEM_OTHER) {
        p = TextCopyUntilEnd(p, g_stat_labels[LABEL_POWER2]);
        p = TextAppendNumber(p, g_item_defs[item].power, 3);
        p = TextCopyUntilEnd(p, g_stat_labels[LABEL_RATE2]);
        p = TextAppendNumber(p, g_item_defs[item].rate, 2);
    } else if (item >= ITEM_ARMOUR) {
        p = TextCopyUntilEnd(p, g_stat_labels[LABEL_POWER]);
        p = TextAppendNumber(p, g_item_defs[item].power, 3);
    } else if (item >= ITEM_WEAPONS) {
        if (item >= ITEM_GUNS) {
            p = TextCopyUntilEnd(p, g_stat_labels[LABEL_POWER]);
            p = TextAppendNumber(p, g_item_defs[item].power, 3);
            p = TextCopyUntilEnd(p, g_stat_labels[LABEL_RATE]);
            p = TextAppendNumber(p, g_item_defs[item].rate, 2);
            p = TextCopyUntilEnd(p, g_stat_labels[g_item_defs[item].hits - LABEL_HITS0]);
        } else {
            p = TextCopyUntilEnd(p, g_stat_labels[LABEL_POWER]);
            p = TextAppendNumber(p, g_item_defs[item].power, 3);
            p = TextCopyUntilEnd(p, g_stat_labels[LABEL_RATE]);
            p = TextAppendNumber(p, g_item_defs[item].rate, 2);
            p = TextCopyUntilEnd(p, g_stat_labels[g_item_defs[item].hits - LABEL_HITS0]);
        }
    } else {
        BgMapInit(g_item_descs[item], 0);
        goto show;
    }
    p[0] = 0xFF;
    p[1] = 1;
    BgMapInit(g_map_name_cells, 0);

show:
    g_bg_layers[LINE_LAYER].x = x;
    g_bg_layers[LINE_LAYER].y = y;
    g_bg_layers[LINE_LAYER].w = LINE_W;
    g_bg_layers[LINE_LAYER].h = LINE_H;
    g_bg_shown |= LINE_BIT;
    return;

clear:
    BgMapClearRow(0);
    BgMapClearRow(1);
    BgMapClearRow(2);
    BgMapClearRow(3);
    m->flags = MSG_DONE;
}
