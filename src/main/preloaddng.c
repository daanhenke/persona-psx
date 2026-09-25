/* Persona 1 (JP) - the DNG preload and the hex formatter beside it.
 *   SLPS_005.00 @ 0x80015020 PreloadDng, 0x80015310 FormatHexDigits
 *
 * A unit of its own, ahead of the other preloads in preload.c. main calls this
 * before handing control to the dungeon overlay, so the CD read is already in
 * flight by the time it starts.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libcd.h>
#include <libsnd.h>
#include <persona/main/cd.h>

extern char *strcpy(char *dst, const char *src);
extern void  FormatHexDigits(int value, char *end, short digits);

extern short  g_map_id[];
extern u_char g_btl_map_id[];
extern int    g_state_next;

/* Where the dungeon was left: the floor, its sub-map, the position and the
   entry the room maps to. PreloadDng saves them on the way in from the field. */
extern u_short g_save_map_id;
/* The same halfword, signed. */
extern short   g_save_floor;
extern u_short g_save_unk4;
extern u_char  g_save_pos_x;
extern u_char  g_save_pos_y;
extern u_char  g_save_unk5_idx;
extern u_char  g_map_pos_x;
extern u_char  g_map_pos_y;

/* Reached by address here, as dng's field.h has the same two bytes. */
#define g_map_unk4 (*(u_char *)0x801F5354)
#define g_map_room (*(u_char *)0x801F5355)

/* How many sub-maps each floor has (0 for none), and the entry each room
   starts from, four bytes a room. */
extern u_char g_dng_floor_maps[];
extern u_char g_dng_room_entry[];

extern short  g_dng_third_gate;
extern u_char g_dng_third_flags;

/* The three DNG filename templates, wrapped in structs so PreloadDng can take
   a fresh copy of one with a plain assignment: "\Dxx\Dyy.BIN;1" is 15 bytes
   with its terminator, the S and M variants 16. */
typedef struct { char c[15]; } DngName15;
typedef struct { char c[16]; } DngName16;
extern const DngName15 str_dng_tmpl;
extern const DngName16 str_dngs_tmpl;
extern const DngName16 str_dngm_tmpl;

/* A floor's sound bank, "\Dxx\DyyA.VB;1" with the letter patched in, and
   the queued form "\Dxx\Dyy_zz.VB;1". */
typedef struct { char c[17]; } DngName17;
extern const DngName15 str_dng_vb_tmpl;
extern const DngName17 str_dng_vbq_tmpl;

/* The three sequences DngSeqMarkCallback hands the music between. */
#define g_seq_handles ((short *)0x801F537C)

#define DNG_VB_BUF ((u_char *)0x80170000)

#define DNG_DEST   ((void *)0x80130000)
#define DNG_M_DEST ((void *)0x801CA000)
#define DNG_S_DEST ((void *)0x801CD000)

/* State 0 (DNG): three filenames of the form \Dxx\Dyy[SM].BIN;1, where yy is
   the floor in hex and xx the floor group (floor >> 3). Each template is
   copied into a local first, so every call starts from a clean one. The third
   file is skipped when entering from ADV with the gate flags clear. */
/* 99.68%. The first FormatHexDigits argument is a fresh `lh` in the image:
   g_save_floor, a signed second name for the halfword (reloc.main.txt pins
   it on the expected side), keeps CSE from folding that read into the
   second test's lhu. The queue submit is two calls that jump2 merges, which
   leaves 3 in the branch's delay slot. What is left is the second test's
   registers: the image has the floor in a0, unk4 in v0 and the table byte in
   v1 - a0 as though the floor were still headed for the call. */
#ifdef NON_MATCHING
void PreloadDng(void)
{
    DngName15 name;
    DngName16 names;
    DngName16 namem;

    name = str_dng_tmpl;
    names = str_dngs_tmpl;
    namem = str_dngm_tmpl;

    /* Coming back from BTLP or a cutscene keeps the floor you were on. */
    if (g_state_next != 1 && g_state_next != 6) {
        g_save_map_id = g_map_id[0];
        g_save_unk4 = g_map_unk4;
        g_save_pos_x = g_map_pos_x;
        g_save_pos_y = g_map_pos_y;
        g_save_unk5_idx = g_dng_room_entry[g_map_room * 4];
    }
    if (g_dng_floor_maps[g_save_map_id] == 0 || g_dng_floor_maps[g_save_map_id] > 0x24) {
        g_save_map_id = 0;
    }
    if (g_save_unk4 >= g_dng_floor_maps[g_save_map_id]) {
        g_save_unk4 = 0;
    }

    FormatHexDigits(g_save_floor, &name.c[7], 2);
    names.c[7] = name.c[7];
    namem.c[7] = name.c[7];
    names.c[6] = name.c[6];
    namem.c[6] = name.c[6];

    FormatHexDigits(g_save_map_id >> 3, &name.c[3], 2);
    g_cd_queue[0].name = name.c;
    g_cd_queue[0].dest = DNG_DEST;
    g_cd_queue[0].mode = 0;
    g_cd_queue[1].name = namem.c;
    g_cd_queue[1].dest = DNG_M_DEST;
    g_cd_queue[1].mode = 0;
    g_cd_queue[2].name = names.c;
    g_cd_queue[2].dest = DNG_S_DEST;
    g_cd_queue[2].mode = 0;

    names.c[3] = name.c[3];
    namem.c[3] = name.c[3];
    names.c[2] = name.c[2];
    namem.c[2] = name.c[2];

    if (g_state_next == 3 && g_dng_third_gate == 0 && (g_dng_third_flags & 1) == 0) {
        CdQueueSubmit(2);
    } else {
        CdQueueSubmit(3);
    }
}
#else
INCLUDE_ASM("main/nonmatchings/preloaddng", PreloadDng);
#endif

/* Writes `digits` hex digits of `value` backwards from `end`, so the caller
   passes a pointer to the *last* digit position. Immediately follows
   PreloadDng in the binary, so it belongs to this translation unit. */
/* 99.48%. Only the digit's register: the image builds it in v0, here in v1.
   Writing it as an if/else gets v0 but moves rem and i down a register. */
#ifdef NON_MATCHING
void FormatHexDigits(int value, char *end, short digits)
{
    short i;
    int   v;
    short rem;
    char  c;

    i = 0;
    if (digits > 0) {
        do {
            v = (short)value;
            value = v / 16;
            rem = v - value * 16;
            c = (char)rem;
            c = c + 0x30;
            if (rem > 9) {
                c = (char)rem + 0x37;
            }
            *end = c;
            i++;
            end--;
        } while (i < digits);
    }
}
#else
INCLUDE_ASM("main/nonmatchings/preloaddng", FormatHexDigits);
#endif

/* Loads floor `floor`'s bank `letter` and transfers its body into VAB *vab. */
void DngLoadVab(short floor, u_char letter, short *vab)
{
    DngName15 name;

    name = str_dng_vb_tmpl;
    FormatHexDigits(floor / 8, &name.c[3], 2);
    FormatHexDigits(floor, &name.c[7], 2);
    name.c[8] = letter;
    LoadFileToAddr(name.c, DNG_VB_BUF);
    SsVabTransBody(DNG_VB_BUF, *vab);
    SsVabTransCompleted(SS_WAIT_COMPLETED);
}

/* Writes a bank's name into queue entry `slot`'s name buffer. Every digit
   pair comes from `floor`; `unused` is not read. */
void DngQueueVabName(short floor, int unused, int slot)
{
    *(DngName17 *)g_cd_queue[slot].name = str_dng_vbq_tmpl;
    FormatHexDigits(floor, (char *)g_cd_queue[slot].name + 3, 2);
    FormatHexDigits(floor, (char *)g_cd_queue[slot].name + 7, 2);
    FormatHexDigits(floor, (char *)g_cd_queue[slot].name + 10, 2);
}

/* Marks 0x6D-0x6F in the dungeon music hand the playback from one of the
   three sequences to the next. */
void DngSeqMarkCallback(short access, short seq, short data)
{
    switch (data) {
    case 0x6D:
        SsSeqStop(g_seq_handles[0]);
        SsSeqSetVol(g_seq_handles[1], 0x7F, 0x7F);
        SsSeqPlay(g_seq_handles[1], SSPLAY_PLAY, 0);
        SsSetMarkCallback(g_seq_handles[0], 0, 0);
        break;
    case 0x6E:
        SsSeqSetVol(g_seq_handles[1], 0, 0);
        SsSeqStop(g_seq_handles[1]);
        SsSeqSetVol(g_seq_handles[2], 0x7F, 0x7F);
        SsPlayBack(g_seq_handles[2], 0, 1);
        SsSetMarkCallback(g_seq_handles[1], 0, 0);
        SsSetMarkCallback(g_seq_handles[2], 0, (SsMarkCallbackProc)DngSeqMarkCallback);
        break;
    case 0x6F:
        SsSeqSetVol(g_seq_handles[0], 0x7F, 0x7F);
        SsSeqPlay(g_seq_handles[0], SSPLAY_PLAY, 0);
        SsSetMarkCallback(g_seq_handles[2], 0, 0);
        break;
    }
}
