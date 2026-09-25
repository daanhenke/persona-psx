/* Persona 1 (JP) - the field's text and event scripts.  DNG only.
 *   0x80073A64 FieldRunScript
 *
 * An event is one script of text and commands, run to its end before the
 * field moves on. Plain bytes are glyphs (a byte with its top bit set takes
 * the next one too), printed a glyph at a time, `g_text_speed` frames apart.
 * 0xFF brings in a code: below 0x20 one of the text codes adv's MsgStep also
 * reads, from 0x20 up one of the event commands its AdvRunScript does. A
 * command that branches keeps the target at +3, as an offset from the
 * script's start less 0x1C0 (the common events hold absolute addresses for
 * their messages).
 *
 * A message (0x55) is text elsewhere in the script: the runner jumps to it,
 * keeping where it was, and text code 1 comes back.
 *
 *  text codes                         event commands
 *   1  back from a message             20  nothing          21  end
 *   2  wait for a button               22  jump             23  jump unless a
 *   3  new line                                                 random byte
 *   4  clear the window                                         reaches n
 *   5  pause, frames16                 24/25  set/clear a story flag
 *   6  text colour                     26  jump if the flag is set
 *   7  a member's surname              2C  go to a map position
 *   8  a character's name              2E  jump if g_script_2B34 is n
 *   9  an item's name                  2F  jump if the member's level < n
 *   10 a spell's name                  30  jump if key is not in the party
 *   11, 12 a Persona's name            3A  jump unless the party holds the item
 *   14 a choice                        46  jump if the member's status is not n
 *   15 a member's first name           49  jump if a stat plus n is below m
 *   16 a member's full name            4D  wait, frames16
 *                                      50  jump if a Persona has that key
 *                                      54  jump if the choice was n
 *                                      55  show a message
 *                                      56  set a flag in the third bank
 *                                      5F  walk the party along a path
 *                                      60/61  set/clear D_8009FDEC
 *
 * Returns the last choice made.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/common/automap.h>
#include <persona/common/char.h>
#include <persona/common/eventflag.h>
#include <persona/common/item.h>
#include <persona/common/persona.h>
#include <persona/common/spell.h>
#include <persona/dng/field.h>

extern int rand(void);

/* The frames between glyphs, from the options. */
extern u_char g_text_speed;
extern u_char g_script_2B34;
extern short  D_8009FADC[];

/* The two events every floor shares (0x100 and 0x101), by address. */
extern u_char *g_common_events[];

/* The paths command 0x5F walks: a facing byte (bit 7 the tune off, 4 in the
   low three bits keeps the facing), then walk-style bytes to 0xFF. */
extern u_char *g_event_walks[];

/* A choice: the text laying it out, its rows and columns, and how many
   cursors it has. */
typedef struct {
    u_char *script;
    u_char  rows;
    u_char  cols;
    u_char  count;
    u_char  pad;
} MsgChoice;
extern MsgChoice g_msg_choices[];

/* Each member's first name and surname, ten bytes each; the hero's are in
   the save area. */
extern u_char g_member_names[][10];
#define HERO_FIRST   ((u_char *)0x801F298C)
#define HERO_SURNAME ((u_char *)0x801F2996)
#define HERO_NAME8   ((u_char *)0x801F29A0)

#define g_flag_bank3 ((u_char *)0x801F2A68)

/* Where 0x2C leads, in the save area (as FloorExit's g_dest_*). */
#define DEST_ID   (*(u_short *)0x801F5350)
#define DEST_X    (*(u_char *)0x801F5352)
#define DEST_Y    (*(u_char *)0x801F5353)
#define DEST_UNK4 (*(u_char *)0x801F5354)
#define DEST_ROOM (*(u_char *)0x801F5355)

#define SCRIPT_CODE 0xFF

#define SEQ_CLICK   3
#define SEQ_CONFIRM 4

#define CHOICE_SPRITE 0x5A

#define WALK_KEEP_FACING 4

/* A branch: the target at +o, an offset from the script's start. */
#define JUMP(o) (s = base + *(int *)(s + (o)) - 0x1C0)

/* Sets the automap bit of the party's tile (x, y). */
static inline void MarkWalked(u_int x, int y)
{
    *(g_map_seen + (g_map_base[g_dng->map] + g_dng->floor) * MAP_BYTES + y * MAP_ROW_BYTES + (x >> 3)) |= 0x80 >> (x & 7);
}

/* Sets story flag n. */
static inline void FlagSet(u_int n)
{
    g_event_flags[n >> 3] |= 1 << (n & 7);
}

/* Sets flag n of the third bank. */
static inline void Bank3Set(u_int n)
{
    g_flag_bank3[n >> 3] |= 1 << (n & 7);
}

int FieldRunScript(int event)
{
    u_char   unused[8]; /* frame space the image reserves and never touches */
    u_char  *ret;
    int      running;
    int      speed;
    u_char  *s;
    u_char  *base;
    u_char  *q;
    int      nowait;  /* never set, but the image tests it every pass */
    int      answer;
    int      row;
    int      i;
    int      n;
    u_char   keep;
    int      pad;
    int      c;
    u_short  w;       /* 0x4D's frames: the u_short copy is the image's temp */
    int      g;
    u_int    f;

    running = 1;
    speed = g_text_speed;
    D_8009FADC[0] = 4;
    D_8009FADC[1] = 0x10;
    g_msg_line = 0;
    g_msg_col = 0;
    g_msg_color = 0;
    FieldMsgClear();
    FieldMsgSetWindow(0);
    FieldFrame();
    nowait = 0;
    g_scene->layers[3].bg.scrolly = 0;
    g_scene->layers[3].bg.scrollx = 0;
    if (event >= 0x100) {
        base = s = g_common_events[event - 0x100];
    } else {
        base = s = (u_char *)(g_floor_event_tab[event] + PACK_BASE);
    }
    g_clock_hold = 1;
    while (running) {
        c = *s;
        if (c == SCRIPT_CODE) {
            s++;
            if (*s < 0x20) {
                switch (*s) {
                case 0:
                    break;
                case 1:
                    s = ret;
                    break;
                case 2:
                    n = 0;
                    while (g_scene->pad_new == 0) {
                        FieldMsgSetWindow(n);
                        FieldFrame();
                        n = (n + 1) % 12;
                    }
                    SsPlayBack(g_seq_handles[SEQ_CONFIRM], 0, 1);
                    FieldMsgSetWindow(0);
                    break;
                case 3:
                    FieldMsgNewLine();
                    break;
                case 4:
                    g_msg_line = 0;
                    g_msg_col = 0;
                    g_scene->layers[3].bg.scrolly = 0;
                    g_scene->layers[3].bg.scrollx = 0;
                    FieldMsgClear();
                    break;
                case 5: {
                    int lo;

                    s++;
                    lo = *s;
                    s++;
                    n = lo + (*s << 8);
                    for (i = 0; i <= n; i += 3) {
                        FieldFrame();
                    }
                    break;
                }
                case 6:
                    s++;
                    g_msg_color = *s;
                    break;
                case 7:
                    s++;
                    n = *s * 2 + 1;
                    if (n == 1) {
                        FieldMsgPrint(HERO_SURNAME, 5);
                    } else {
                        FieldMsgPrint(g_member_names[n], 5);
                    }
                    break;
                case 8:
                    s++;
                    if (*s++ == 0) {
                        FieldMsgPrintBytes(HERO_NAME8, 10);
                    }
                    FieldMsgPrintBytes(g_char_templates[*s].name, 10);
                    break;
                case 9:
                    FieldMsgPrintBytes(g_item_defs[s[1] + (s[2] << 8)].name, 10);
                    s += 2;
                    break;
                case 10:
                    s++;
                    FieldMsgPrintBytes(g_spell_data[*s].name, 10);
                    break;
                case 11:
                case 12:
                    s++;
                    FieldMsgPrintBytes(g_persona_data[*s].name, 10);
                    break;
                case 14:
                    s++;
                    FieldMsgPrintCodes(g_msg_choices[*s].script, 0xFF);
                    q = s;
                    for (i = 0; i < (D_8009FE3C = g_msg_choices[*q].count); i++) {
                        FieldInitSprite(i + CHOICE_SPRITE, 0x10, 0x10, 0x1E, 0, 0xB4, 0x100, 0x1F8);
                        g_scene->sprites[CHOICE_SPRITE + i].attribute = 0x40000000;
                    }
                    answer = 0;
                    row = 0;
                    n = 0;
                    for (;;) {
                        pad = g_scene->pad_new;
                        if (pad & BIND(lift_ok)) {
                            SsPlayBack(g_seq_handles[SEQ_CONFIRM], 0, 1);
                            break;
                        }
                        if (pad & PAD_UP) {
                            SsPlayBack(g_seq_handles[SEQ_CLICK], 0, 1);
                            row = row - 1 < 0 ? g_msg_choices[*q].rows - 1 : row - 1;
                        } else if (pad & PAD_DOWN) {
                            SsPlayBack(g_seq_handles[SEQ_CLICK], 0, 1);
                            row = (row + 1) % g_msg_choices[*q].rows;
                        } else if (pad & PAD_LEFT) {
                            SsPlayBack(g_seq_handles[SEQ_CLICK], 0, 1);
                            answer = answer - 1 < 0 ? g_msg_choices[*q].cols - 1 : answer - 1;
                        } else if (pad & PAD_RIGHT) {
                            SsPlayBack(g_seq_handles[SEQ_CLICK], 0, 1);
                            answer = (answer + 1) % g_msg_choices[*q].cols;
                        }
                        /* g_scene in a local, so the sprite's address is one
                           sum, g_scene first, shared by every store. */
                        for (i = 0; i < D_8009FE3C; i++) {
                            DngScene *g = g_scene;
                            GsSPRITE *sp = &g->sprites[CHOICE_SPRITE + i];

                            sp->x = answer * g_msg_choices[*q].count * 16 + ((i - 8) << 4);
                            sp->y = row * 16 + 0x40;
                            sp->r = sp->g = sp->b = n < 0x80 ? n : -n;
                        }
                        FieldFrame();
                        n = (n + 0x18) & 0xFF;
                    }
                    D_8009FE3C = 0;
                    answer += row * g_msg_choices[*q].cols;
                    s = ret;
                    break;
                case 15:
                    s++;
                    n = *s * 2;
                    if (n == 0) {
                        FieldMsgPrint(HERO_FIRST, 5);
                    } else {
                        FieldMsgPrint(g_member_names[n], 5);
                    }
                    break;
                case 16:
                    s++;
                    n = *s * 2;
                    if (n == 0) {
                        FieldMsgPrint(HERO_FIRST, 5);
                        FieldMsgPutGlyph(0);
                        FieldFrame();
                        FieldMsgPrint(HERO_SURNAME, 5);
                    } else {
                        FieldMsgPrint(g_member_names[n], 5);
                        FieldMsgPutGlyph(0);
                        FieldFrame();
                        FieldMsgPrint(g_member_names[n + 1], 5);
                    }
                    break;
                }
                s++;
            } else {
                switch (*s) {
                case 0x21:
                    running = 0;
                    /* fall through */
                case 0x20:
                    s += 3;
                    break;
                case 0x23:
                    s++;
                    /* The roll lands in i, as the image's slt does. */
                    i = (u_char)(*s - 1);
                    i = i < (rand() & 0xFF);
                    if (!i) {
                        JUMP(2);
                    } else {
                        s += 6;
                    }
                    break;
                case 0x24:
                    FlagSet(*(u_short *)(s + 1));
                    s += 3;
                    break;
                case 0x25:
                    f = *(u_short *)(s + 1);
                    g_event_flags[f >> 3] &= ~(1 << (f & 7));
                    s += 3;
                    break;
                case 0x26:
                    f = *(u_short *)(s + 1);
                    if ((g_event_flags[f >> 3] >> (f & 7)) & 1) {
                        JUMP(3);
                        break;
                    }
                    s += 7;
                    break;
                case 0x2C:
                    DEST_ID = s[1];
                    DEST_UNK4 = s[2];
                    DEST_X = s[3];
                    DEST_Y = s[4];
                    DEST_ROOM = s[5];
                    FieldFadeOut();
                    g_dng->map = DEST_ID;
                    g_dng->floor = DEST_UNK4;
                    g_dng->pos[POS_X] = DEST_X;
                    g_dng->pos[POS_Y] = DEST_Y;
                    s += 7;
                    g_dng->facing = g_dest_facings[DEST_ROOM][0];
                    FieldSetFloor();
                    FieldSetupGfx(0);
                    FieldSyncMusic();
                    g_field_lit = 0;
                    for (i = 0; i < 3; i++) {
                        FieldFrame();
                    }
                    FieldFadeIn();
                    break;
                case 0x2E:
                    if (s[1] == g_script_2B34) {
                        JUMP(3);
                        break;
                    }
                    s += 7;
                    break;
                case 0x2F:
                    n = FieldFindMember(s[1]);
                    if (n != -1 && g_chars[n].level < s[2]) {
                        JUMP(3);
                        break;
                    }
                    s += 7;
                    break;
                case 0x30:
                    if (FieldFindMember(s[1]) == -1) {
                        JUMP(3);
                        break;
                    }
                    s += 7;
                    break;
                case 0x3A:
                    n = 0;
                    for (i = 0; i < 0x17F; i++) {
                        if ((g_items[i] & 0x1FF) == *(u_short *)(s + 1)) {
                            n = 1;
                            break;
                        }
                    }
                    if (!n) {
                        JUMP(3);
                        break;
                    }
                    s += 7;
                    break;
                case 0x46:
                    n = FieldFindMember(s[1]);
                    if (n != -1 && g_chars[n].status != s[2]) {
                        JUMP(3);
                        break;
                    }
                    s += 7;
                    break;
                case 0x49:
                    n = FieldFindMember(s[1]);
                    if (n != -1 && g_chars[n].stat[0] + s[2] < s[3]) {
                        JUMP(7);
                        break;
                    }
                    s += 11;
                    break;
                case 0x4D:
                    s++;
                    w = *(u_short *)s;
                    for (i = 0; i <= w; i += 3) {
                        FieldFrame();
                    }
                    s += 2;
                    break;
                case 0x50:
                    n = 0;
                    for (i = 0; i < 31; i++) {
                        if (g_personas[i].key == s[1]) {
                            n = 1;
                            break;
                        }
                    }
                    if (n) {
                        JUMP(3);
                        break;
                    }
                    s += 7;
                    break;
                case 0x54:
                    if (answer == s[1]) {
                        JUMP(3);
                        break;
                    }
                    s += 7;
                    break;
                case 0x55:
                    g_msg_line = 0;
                    g_msg_col = 0;
                    FieldMsgClear();
                    ret = s + 6;
                    if (event >= 0x100) {
                        s = *(u_char **)(s + 3);
                    } else {
                        JUMP(3);
                    }
                    break;
                case 0x22:
                    JUMP(3);
                    break;
                case 0x56:
                    Bank3Set(s[1]);
                    s += 3;
                    break;
                case 0x5F:
                    n = s[1];
                    g_bgm_off = g_event_walks[n][0] & 0x80;
                    keep = g_dng->no_enc;
                    g_dng->no_enc = 1;
                    if ((g_event_walks[n][0] & 7) != WALK_KEEP_FACING) {
                        while ((g_event_walks[n][0] & 3) != g_dng->facing) {
                            if (((g_dng->facing + 3) & 3) != (g_event_walks[n][0] & 3)) {
                                g_scene->pad_held = PAD_RIGHT;
                            } else {
                                g_scene->pad_held = PAD_LEFT;
                            }
                            g_scene->pad_new = 0;
                            FieldUpdate(0);
                            FieldFrame();
                        }
                    }
                    for (i = 1; g_event_walks[n][i] != 0xFF; ) {
                        FieldMsgSetStyle(g_event_walks[n][i]);
                        i++;
                        while (D_8009FAE0 != 0) {
                            g_scene->pad_new = 0;
                            g_scene->pad_held = D_8009FAE4;
                            FieldUpdate(0);
                            FieldFrame();
                            MarkWalked(g_dng->pos[POS_X], g_dng->pos[POS_Y]);
                            D_8009FAE0--;
                        }
                    }
                    g_dng->no_enc = keep;
                    g_bgm_off = 0;
                    s += 3;
                    break;
                case 0x60:
                    D_8009FDEC = 1;
                    s += 3;
                    FieldMsgClear();
                    break;
                case 0x61:
                    s += 3;
                    D_8009FDEC = 0;
                    break;
                }
            }
        } else if (!(*s & 0x80)) {
            s++;
            FieldMsgPutGlyph(c);
            if (speed) {
                for (i = 0; i < speed; i++) {
                    FieldFrame();
                }
            }
        } else {
            g = ((*s & 0x7F) << 8) | s[1];
            s += 2;
            FieldMsgPutGlyph(g);
            if (speed) {
                for (i = 0; i < speed; i++) {
                    FieldFrame();
                }
            }
        }
        if (!nowait) {
            FieldFrame();
        }
        nowait = 0;
    }
    g_clock_hold = 0;
    return answer;
}
