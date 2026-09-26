/* Persona 1 (JP) - setting the name screen up.  NAME @ 0x800670F8.
 *
 * The loader leaves the screen's files at 0x80140000, each reached by an
 * offset in the table at the start of the buffer: the sound bank and its six
 * sequences, the font's palette and image, the four backgrounds and the
 * sprite sheet.
 */
#include <decomp/types.h>
#include <libetc.h>
#include <libsnd.h>
#include <persona/name/entry.h>

void UploadImageOne(int *p, u_short x, u_short y);
void UploadImageRows(void *desc, u_short x, u_short y, short rows);
void TimLoad(u_long *tim, int nopal);
void BgFromPack(u_long *pack, u_char *hdr, GsMAP *map, GsBG *bg, short x,
                short y);

/* File `n` of the load buffer. */
#define PACK_FILE(n) ((u_char *)NAME_PACK + NAME_PACK[n])

void NameEntryInit(void)
{
    RECT r;
    int  i;

    SetDispMask(0);
    ResetCallback();
    SsEnd();
    SsQuit();
    SsInit();
    ResetGraph(1);
    GsInitGraph(0x140, 0xF0, 4, 0, 0);
    r.x = 0;
    r.y = 0;
    r.w = 0x3FF;
    r.h = 0x1FF;
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
    GsDefDispBuff(0, 0, 0, 0xF0);
    SetDispMask(1);
    SsSetStereo();
    SsUtSetReverbType(4);
    SsUtReverbOn();
    SsSetTickMode(1);
    SsSetTableSize((char *)0x801F0000, 0x20, 1);
    g_vab_id = SsVabOpenHead(PACK_FILE(13), -1);
    SsVabTransBody(PACK_FILE(14), g_vab_id);
    SsVabTransCompleted(1);
    for (i = 0; i < 6; i++) {
        g_seq_handles[i] = SsSeqOpen((u_long *)PACK_FILE(15 + i), g_vab_id);
        SsSeqSetVol(g_seq_handles[i], 0x7F, 0x7F);
    }
    SsStart();
    SsSetMVol(0x7F, 0x7F);
    VSync(0x3C);
    SsUtSetReverbDepth(0x40, 0x40);
    SsSeqPlay(g_seq_handles[0], 1, 0);
    UploadImageOne((int *)PACK_FILE(12), 0x180, 0);
    UploadImageRows(PACK_FILE(0), 0, 0x1E0, 3);
    TimLoad((u_long *)PACK_FILE(11), 0);
    DrawSync(0);
    for (i = 0; i < 6; i++) {
        ExpandGlyph(g_name_labels[i], &g_glyph_cell[i * 2], 12);
    }
    UploadImage(0x200, 0, 0x18, 0x10, g_glyph_cell);
    NameDrawKeyboardPage(g_name_page);
    g_ot[1].length = 4;
    g_ot[0].length = 4;
    g_ot[0].org = (GsOT_TAG *)g_ot_tags[0];
    g_ot[1].org = (GsOT_TAG *)g_ot_tags[1];
    g_pad_trig = 0;
    g_pad_raw = 0;
    g_pad_held = 0;
    BgFromPack((u_long *)PACK_FILE(3), PACK_FILE(4), &g_map_list, &g_bg_list, 8, 0x10);
    BgFromPack((u_long *)PACK_FILE(5), PACK_FILE(6), &g_map_keys, &g_bg_keys, 8, 0x68);
    BgFromPack((u_long *)PACK_FILE(7), PACK_FILE(8), &g_map_frame, &g_bg_frame, 0x58, 0x78);
    g_bg_frame.attribute = 0;
    g_bg_frame.h = 0x60;
    BgFromPack((u_long *)PACK_FILE(9), PACK_FILE(10), &g_map_fields, &g_bg_fields, 0x48, 0x78);
    g_bg_fields.attribute = 0;
    g_bg_fields.h = 0x60;
    for (i = 0; i < NAME_SPRITES; i++) {
        NameSpriteFromDef(i);
    }
    g_sprites[SPR_SIDE_CARET].attribute |= 0x40000000;
    g_sprites[SPR_PAGE_TAB].scalex = 0x1400;
    g_sprites[SPR_PAGE_TAB].scaley = 0x1200;
    g_sprites[SPR_PAGE_TAB2].scalex = 0x1200;
    g_sprites[SPR_PAGE_TAB2].scaley = 0x1200;
    g_sprites[11].scalex = 0x1400;
    g_sprites[11].scaley = 0x1200;
    g_sprites[SPR_KEY_CARET].scalex = 0x1600;
    g_sprites[SPR_KEY_CARET].scaley = 0x1600;
    DrawSync(0);
}
