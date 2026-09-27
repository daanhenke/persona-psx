/* Persona 1 (JP) - END.EXE, the ending.  @ 0x80080EF8
 *
 * main runs the credits script of the ending the resident asked for: it
 * streams each credit image in from END.BIN into one of two sprite pages,
 * fades the pages in and out, plays the ending's music and movies between
 * rolls, and returns to the resident with the scene it goes on to. The movie
 * player is the same streaming sample MOVIE.EXE carries.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libcd.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/common/eventflag.h>
#include <persona/common/str.h>
#include <persona/end/end.h>

extern int  StCdIntrFlag;
extern void StCdInterrupt(void);
extern void SsSetStereo(void);
extern void SsUtReverbOn(void);
extern void SsSeqClose(short seq);
extern void SsQuit(void);

/* Where the game resumes, reached by literal address. */
#define MAP_ID   (*(u_short *)0x801F5350)
#define MAP_ROOM (*(u_char *)0x801F5355)

void StrOutCallback(void);

/* Defined here: the unit reaches them gp-relative. */
int      g_end_pad;
int      g_end_pad_old;
int      g_end_pad_trig;
int      g_end_show_sprite;
int      g_end_movie_frame;
int      g_end_buf;
short    g_end_vab;
short    g_end_seq;
u_short *g_end_script;
u_char   g_end_image_kind;
int      g_end_base;
int      g_end_page_a;
int      g_end_movie;
int      g_end_page_b;
int      g_end_page_c;
int      g_end_movie_done;
CdlLOC   g_str_loc;
volatile int g_cd_busy;

/* Where each credit image, song and movie starts in END.BIN, in sectors
   from the start of the file; each runs up to the next entry. */
u_short g_end_sectors[] = {
      0x1,  0x4D,  0x60,  0xAC,  0xBF, 0x10B, 0x11E, 0x13C,
    0x14F, 0x16E, 0x181, 0x19F, 0x1B2, 0x1D0, 0x1E3, 0x202,
    0x215, 0x233, 0x246, 0x265, 0x278, 0x2C4, 0x2D7, 0x323,
    0x336, 0x382, 0x395, 0x3A8, 0x3F4, 0x407, 0x453, 0x466,
    0x485, 0x492, 0x49F, 0x4AC, 0x4B9, 0x4C6, 0x4D3, 0x4E0,
    0x4ED, 0x4FA, 0x507, 0x514, 0x521, 0x52E, 0x53B, 0x548,
    0x555, 0x562, 0x56F, 0x57C, 0x589, 0x596, 0x5A3, 0x5B0,
    0x5BD, 0x5D0, 0x5E3, 0x5F6, 0x609, 0x61C, 0x62F, 0x642,
    0x655, 0x668, 0x67B, 0x68E, 0x6A1, 0x6B4, 0x6C5, 0x6C9,
    0x7B7, 0x7C0, 0x7C2, 0x833, 0x836, 0x83B, 0x8FB, 0x913,
    0x915, 0x92E, 0x92F, 0x931, 0x9A2, 0x9A4,
};

/* How many frames each movie runs. */
u_short g_end_movie_frames[] = {
     0x52,  0x52,  0x52,  0x52,  0x52,  0x52,  0x52,  0x5B,
     0x52,  0x52,  0x52,  0x68,  0xBE,  0xE2,  0xE2,  0xE2,
     0xE2, 0x3C1, 0x4CE, 0x2B2, 0x384, 0x31B, 0x330, 0x2FD,
     0xCF, 0x144,  0xC8, 0x4BF, 0x82A, 0x177, 0x37A, 0x1A4,
    0x195, 0x197, 0x11D, 0x11D, 0x136, 0x21F, 0x1F9,
};

/* The credits script of each ending: an opcode, then its argument where it
   takes one (see main). */
u_short g_end_script0[] = {
      0x7,  0x1B,   0x0,   0x0,   0x6,   0x3,   0x1,   0x2,
      0x1,   0x5,  0x2D,   0x3,   0x4,   0x0,   0x2,   0x5,
    0x10E,   0x2,   0x3,   0x4,   0x5,   0x5,  0x2D,   0x3,
      0x1,   0x5,  0x2D,   0x3,   0x4,   0x5, 0x10E,   0x4,
      0x5,   0x7,  0x25,   0x9,
};

u_short g_end_script1[] = {
      0x7,  0x26,   0xB,  0x46,   0x0,   0x4,   0x6,   0x1,
      0x6,   0x3,   0x1,   0x5,  0x2D,   0x2,   0x5,   0x3,
      0x2,   0x5,  0x2D,   0x3,   0x4,   0x1,   0x8,   0x5,
    0x258,   0x4,   0x6,   0x2,   0x7,   0x5,  0x2D,   0x3,
      0x2,   0x5,  0x2D,   0x3,   0x4,   0x1,   0xA,   0x5,
    0x258,   0x4,   0x6,   0x2,   0x9,   0x5,  0x2D,   0x3,
      0x2,   0x5,  0x2D,   0x3,   0x4,   0x1,   0xC,   0x5,
    0x258,   0x4,   0x6,   0x2,   0xB,   0x5,  0x2D,   0x3,
      0x2,   0x5,  0x2D,   0x3,   0x4,   0x1,  0x1F,   0x5,
    0x258,   0x4,   0x6,   0x2,  0x1C,   0x5,  0x2D,   0x3,
      0x2,   0x5,  0x2D,   0x3,   0x4,   0x0,  0x20,   0x5,
    0x258,   0x4,   0x6,   0x5,  0x5A,   0x4,   0x1,   0x5,
     0x2D,   0x8,  0x46,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x21,   0x5,  0x96,   0x4,   0x5,   0x5,
     0x2D,   0x8,  0x47,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x22,   0x5,  0x96,   0x4,   0x5,   0x2,
     0x38,   0x5,  0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x23,   0x5,  0x96,   0x4,   0x5,   0x2,
     0x39,   0x5,  0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x5,  0x96,   0x4,   0x4,   0x2,  0x3A,   0x5,
     0x2D,   0x3,   0x4,   0x0,  0x24,   0x5,  0x96,   0x4,
      0x5,   0x2,  0x3B,   0x5,  0x2D,   0x3,   0x1,   0x5,
     0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,   0x4,   0x2,
     0x3C,   0x5,  0x2D,   0x3,   0x4,   0x0,  0x25,   0x5,
     0x96,   0x4,   0x5,   0x2,  0x3D,   0x5,  0x2D,   0x3,
      0x1,   0x5,  0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,
      0x4,   0x2,  0x3E,   0x5,  0x2D,   0x3,   0x4,   0x0,
     0x26,   0x5,  0x96,   0x4,   0x5,   0x2,  0x3F,   0x5,
     0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,   0x4,   0x5,
     0x96,   0x4,   0x4,   0x2,  0x40,   0x5,  0x2D,   0x3,
      0x4,   0x5,  0x96,   0x4,   0x5,   0x2,  0x41,   0x5,
     0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,   0x4,   0x2,
     0x42,   0x5,  0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,
      0x4,   0x2,  0x43,   0x5,  0x2D,   0x3,   0x4,   0x5,
     0x96,   0x4,   0x4,   0x2,  0x44,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x27,   0x5,  0x96,   0x4,   0x4,   0x8,
     0x48,   0x5,  0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x5, 0x10E,   0x4,   0x5,   0x8,  0x49,   0x5,
     0x2D,   0x3,   0x4,   0x5, 0x32A,   0x8,  0x4A,   0x3,
      0x8,   0xA,
};

u_short g_end_script2[] = {
      0x0,   0x0,   0x6,   0x3,   0x1,   0x2,   0x1,   0x5,
     0x2D,   0x0,   0x2,   0x3,   0x4,   0x2,   0x3,   0x5,
    0x10E,   0x4,   0x5,   0x5,  0x2D,   0x3,   0x1,   0x5,
     0x2D,   0x3,   0x4,   0x5, 0x10E,   0x4,   0x5,   0x7,
     0x1E,   0xB,  0x49,   0x8,  0x46,   0x3,   0x4,   0x5,
     0x96,   0x4,   0x4,   0x5,  0x2D,   0x8,  0x47,   0x3,
      0x4,   0x5,  0x96,   0x4,   0x4,   0x2,  0x38,   0x5,
     0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,   0x4,   0x2,
     0x39,   0x5,  0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,
      0x4,   0x2,  0x3A,   0x5,  0x2D,   0x3,   0x4,   0x5,
     0x96,   0x4,   0x4,   0x2,  0x3B,   0x5,  0x2D,   0x3,
      0x4,   0x5,  0x96,   0x4,   0x4,   0x2,  0x3C,   0x5,
     0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,   0x4,   0x2,
     0x3D,   0x5,  0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,
      0x4,   0x2,  0x3E,   0x5,  0x2D,   0x3,   0x4,   0x5,
     0x96,   0x4,   0x4,   0x2,  0x3F,   0x5,  0x2D,   0x3,
      0x4,   0x5,  0x96,   0x4,   0x4,   0x2,  0x40,   0x5,
     0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,   0x4,   0x2,
     0x41,   0x5,  0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,
      0x4,   0x2,  0x42,   0x5,  0x2D,   0x3,   0x4,   0x5,
     0x96,   0x4,   0x4,   0x2,  0x43,   0x5,  0x2D,   0x3,
      0x4,   0x5,  0x96,   0x4,   0x4,   0x2,  0x44,   0x5,
     0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,   0x4,   0x8,
     0x48,   0x5,  0x2D,   0x3,   0x4,   0x5, 0x10E,   0x4,
      0x4,   0x8,  0x49,   0x5,  0x2D,   0x3,   0x4,   0x5,
    0x32A,   0x8,  0x4A,   0x3,   0x8,   0xA,
};

u_short g_end_script3[] = {
      0xB,  0x4C,   0x0,  0x14,   0x6,   0x2,  0x13,   0x3,
      0x1,   0x5,  0x2D,   0x3,   0x4,   0x5, 0x10E,   0x2,
     0x15,   0x4,   0x4,   0x5,  0x2D,   0x3,   0x4,   0x5,
    0x10E,   0x2,  0x17,   0x4,   0x4,   0x5,  0x2D,   0x0,
     0x28,   0x3,   0x4,   0x5, 0x10E,   0x4,   0x5,   0x5,
     0x2D,   0x8,  0x46,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x29,   0x5,  0x96,   0x4,   0x5,   0x5,
     0x2D,   0x8,  0x47,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x2A,   0x5,  0x96,   0x4,   0x5,   0x2,
     0x38,   0x5,  0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x2B,   0x5,  0x96,   0x4,   0x5,   0x2,
     0x39,   0x5,  0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x5,  0x96,   0x4,   0x4,   0x2,  0x3A,   0x5,
     0x2D,   0x3,   0x4,   0x0,  0x2C,   0x5,  0x96,   0x4,
      0x5,   0x2,  0x3B,   0x5,  0x2D,   0x3,   0x1,   0x5,
     0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,   0x4,   0x2,
     0x3C,   0x5,  0x2D,   0x3,   0x4,   0x0,  0x2D,   0x5,
     0x96,   0x4,   0x5,   0x2,  0x3D,   0x5,  0x2D,   0x3,
      0x1,   0x5,  0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,
      0x4,   0x2,  0x3E,   0x5,  0x2D,   0x3,   0x4,   0x0,
     0x2E,   0x5,  0x96,   0x4,   0x5,   0x2,  0x3F,   0x5,
     0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,   0x4,   0x5,
     0x96,   0x4,   0x4,   0x2,  0x40,   0x5,  0x2D,   0x3,
      0x4,   0x5,  0x96,   0x4,   0x5,   0x2,  0x41,   0x5,
     0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,   0x4,   0x2,
     0x42,   0x5,  0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,
      0x4,   0x2,  0x43,   0x5,  0x2D,   0x3,   0x4,   0x5,
     0x96,   0x4,   0x4,   0x2,  0x44,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x2F,   0x5,  0x96,   0x4,   0x4,   0x8,
     0x48,   0x5,  0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x5, 0x10E,   0x4,   0x5,   0x8,  0x49,   0x5,
     0x2D,   0x3,   0x4,   0x5, 0x32A,   0x8,  0x4A,   0x3,
      0x8,   0x5, 0x32A,   0x9,
};

u_short g_end_script4[] = {
      0xB,  0x4F,   0x0,  0x1B,   0x6,   0x2,  0x19,   0x3,
      0x1,   0x5,  0x2D,   0x3,   0x4,   0x0,  0x1D,   0x5,
    0x10E,   0x4,   0x5,   0x2,  0x1A,   0x5,  0x2D,   0x3,
      0x1,   0x2,  0x1E,   0x5,  0x2D,   0x3,   0x4,   0x5,
    0x10E,   0x4,   0x4,   0x5,  0x2D,   0x0,  0x30,   0x3,
      0x4,   0x5, 0x10E,   0x4,   0x5,   0xB,  0x52,   0x5,
     0x2D,   0x8,  0x46,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x31,   0x5,  0x96,   0x4,   0x5,   0x5,
     0x2D,   0x8,  0x47,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x32,   0x5,  0x96,   0x4,   0x5,   0x2,
     0x38,   0x5,  0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x33,   0x5,  0x96,   0x4,   0x5,   0x2,
     0x39,   0x5,  0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x5,  0x96,   0x4,   0x4,   0x2,  0x3A,   0x5,
     0x2D,   0x3,   0x4,   0x0,  0x34,   0x5,  0x96,   0x4,
      0x5,   0x2,  0x3B,   0x5,  0x2D,   0x3,   0x1,   0x5,
     0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,   0x4,   0x2,
     0x3C,   0x5,  0x2D,   0x3,   0x4,   0x0,  0x35,   0x5,
     0x96,   0x4,   0x5,   0x2,  0x3D,   0x5,  0x2D,   0x3,
      0x1,   0x5,  0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,
      0x4,   0x2,  0x3E,   0x5,  0x2D,   0x3,   0x4,   0x0,
     0x36,   0x5,  0x96,   0x4,   0x5,   0x2,  0x3F,   0x5,
     0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,   0x4,   0x5,
     0x96,   0x4,   0x4,   0x2,  0x40,   0x5,  0x2D,   0x3,
      0x4,   0x5,  0x96,   0x4,   0x5,   0x2,  0x41,   0x5,
     0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,   0x4,   0x2,
     0x42,   0x5,  0x2D,   0x3,   0x4,   0x5,  0x96,   0x4,
      0x4,   0x2,  0x43,   0x5,  0x2D,   0x3,   0x4,   0x5,
     0x96,   0x4,   0x4,   0x2,  0x44,   0x5,  0x2D,   0x3,
      0x4,   0x0,  0x37,   0x5,  0x96,   0x4,   0x4,   0x8,
     0x48,   0x5,  0x2D,   0x3,   0x1,   0x5,  0x2D,   0x3,
      0x4,   0x5, 0x10E,   0x4,   0x5,   0x7,  0x1F,   0x9,
};

u_short *g_end_scripts[] = {
    g_end_script0, g_end_script1, g_end_script2, g_end_script3, g_end_script4,
};

/* The layout each credit image uses, and where each layout puts it. */
u_char g_end_image_kinds[] = {
     0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x2,  0x2,  0x1,  0x1,  0x2,  0x1,
     0x2,  0x2,  0x1,  0x1,  0x2,  0x2,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,
     0x0,  0x0,  0x0,  0x0,  0x2,  0x0,  0x0,  0x1,  0x7,  0x3,  0x6,  0x4,
     0x5,  0x3,  0x4,  0x7,  0x7,  0x3,  0x6,  0x4,  0x5,  0x3,  0x4,  0x7,
     0x7,  0x3,  0x6,  0x4,  0x5,  0x3,  0x4,  0x7,  0x0,  0x0,  0x0,  0x0,
     0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x8,  0x9,
     0xA,  0xB,  0xC,  0x0,
};

u_short g_end_kind_x[] = {
      0x0,  0x10,  0xAF,  0x1E,  0x1E,  0x9C,  0x9C,
     0x5D,  0x6E,  0xA6,  0x74,  0x52, 0x100,   0x0,
};

u_short g_end_kind_y[] = {
      0x0,  0x10,  0x10,  0x1E,  0x6E,  0x1E,  0x6E,
     0x1E,  0x88,  0x88,  0x8C,  0x69,  0xBE,   0x0,
};

/* The part of the credits frame each logo shows. */
u_char g_end_logo_u[] = { 0x0, 0x0, 0x68, 0x0, 0x68, 0x0, 0x0, 0x0 };
u_char g_end_logo_v[] = { 0x0, 0x30, 0x0, 0x60, 0x30, 0x0, 0x0, 0x0 };
u_char g_end_logo_w[] = { 0x68, 0x68, 0x60, 0xA0, 0x20, 0x0, 0x0, 0x0 };
u_char g_end_logo_h[] = { 0x30, 0x30, 0x30, 0x28, 0x18, 0x0, 0x0, 0x0 };

#ifdef NON_MATCHING
/* Runs the ending's credits script to its end and returns to the resident
   with the scene it goes on to. */
int main(void)
{
    CdlLOC  pos;
    u_char  param;
    int     ending;
    int     load;
    int     fade;
    int     wait;
    int     level;
    int     page_a;
    int     page_b;
    int     step;
    int     page_c;
    int     n;
    int     a;
    int     b;

    ResetCallback();
    SetDispMask(0);
    SsEnd();
    SsQuit();
    SsInit();
    g_end_vab = g_end_seq = -1;
    while (!CdInit()) {
    }
    ResetGraph(0);
    PadInit(0);
    GsInitGraph(320, 240, 4, 0, 0);
    ending = g_movie_id - 0x30;
    g_end_script = g_end_scripts[ending];
    while (!CdSearchFile(&g_end_file, "\\EXE\\END.BIN;1")) {
    }
    g_end_base = CdPosToInt(&g_end_file.pos);
    load   = 0;
    fade   = 0;
    page_a = 0;
    page_c = 0;
    wait   = 0;
    page_b = 0;
    g_cd_busy = -1;
    g_end_page_a = 0;
    g_end_page_b = 0;
    g_end_page_c = 0;

    /* The frame the credits are drawn in. */
    CdIntToPos(g_end_sectors[0x45] + g_end_base, &pos);
    CdReadFileToAddr(&pos, g_end_sectors[0x46] - g_end_sectors[0x45], (u_long *)0x80180000);
    EndLoadTim((u_long *)0x80180004, 0, 0x3C0, 2);
    DrawSync(0);
    g_end_image.ph = 24;
    g_end_image.cy = 0x1E2;
    g_end_image.cx = 0;
    EndSetSprite(&g_end_sprite, 32, 0x80, 256, 190, 15);
    g_end_sprite.u = 0x68;
    g_end_sprite.v = 0x30;

    /* Which cast the last rolls show depends on how the story went. */
    a = 0;
    b = 1;
    if (ending == 1 && !(g_event_flags[5] & 1)) {
        if (g_event_flags[5] & 2) {
            a = 0xE;
            b = 0xD;
        } else if (g_event_flags[5] & 4) {
            a = 0x10;
            b = 0xF;
        } else if (g_event_flags[6] & 0x20) {
            a = 0x12;
            b = 0x11;
        }
        g_end_script1[0x36] = a;
        g_end_script1[0x3C] = b;
    } else if (ending == 3 && !(g_event_flags[0x24] & 2)) {
        if (g_event_flags[0x24] & 1) {
            a = 0x16;
        }
        if (g_event_flags[0x24] & 4) {
            a = 0x18;
        }
        g_end_script3[3] = a;
    }

    EndInit();
    for (;;) {
        switch (*g_end_script) {
        case 0:
            if (g_cd_busy == -1 && load == 0) {
                EndLoadImage();
                load = 1;
                g_end_script++;
            }
            break;
        case 1:
            if (g_cd_busy == -1 && load == 0) {
                EndLoadImage();
                load = 2;
                g_end_script++;
            }
            break;
        case 2:
            if (g_cd_busy == -1 && load == 0) {
                EndLoadImage();
                load = 3;
                g_end_script++;
            }
            break;
        case 3:
            if (wait == 0 && fade == 0) {
                fade = *++g_end_script;
                if (fade & 1) {
                    g_end_spr_a[g_end_page_a][0].attribute &= 0x7FFFFFFF;
                    if (g_end_spr_a[g_end_page_a][1].x) {
                        g_end_spr_a[g_end_page_a][1].attribute &= 0x7FFFFFFF;
                    }
                }
                if (fade & 2) {
                    g_end_spr_b[g_end_page_b].attribute &= 0x7FFFFFFF;
                }
                if (fade & 4) {
                    g_end_spr_c[g_end_page_c][0].attribute &= 0x7FFFFFFF;
                    if (g_end_spr_c[g_end_page_c][1].x) {
                        g_end_spr_c[g_end_page_c][1].attribute &= 0x7FFFFFFF;
                    }
                }
                if (fade & 8) {
                    g_end_sprite.attribute &= 0x7FFFFFFF;
                }
                level = 0;
                step = 1;
                g_end_script++;
            }
            break;
        case 4:
            if (wait == 0 && fade == 0) {
                step  = -1;
                fade  = *++g_end_script;
                level = 0x80;
                g_end_script++;
            }
            break;
        case 5:
            if (fade == 0 && wait == 0) {
                wait = *++g_end_script;
                g_end_script++;
            }
            break;
        case 6:
            if (fade == 0 && wait == 0 && g_cd_busy == -1) {
                g_end_script++;
            }
            break;
        case 7:
            if (fade == 0 && wait == 0) {
                g_end_movie = *++g_end_script;
                EndPlayMovie();
                param = CdlModeSpeed;
                while (!CdControlB(CdlSetmode, &param, 0)) {
                }
                DecDCToutCallback(0);
                CdDataCallback(0);
                CdReadyCallback(0);
                while (!CdControlB(CdlPause, 0, 0)) {
                }
                EndInit();
                g_end_script++;
            }
            break;
        case 8:
            n = *++g_end_script;
            g_end_image_kind = g_end_image_kinds[n];
            if (n != 0x4A) {
                EndSetSprite(&g_end_spr_c[page_c][1], 0, 0x80, 0, 0, 0);
                EndSetSprite(&g_end_spr_c[page_c][0], g_end_image.pw * 4, 0x80,
                             g_end_kind_x[g_end_image_kind], g_end_kind_y[g_end_image_kind], 15);
                n -= 0x46;
                g_end_spr_c[page_c][0].u  = g_end_logo_u[n];
                g_end_spr_c[page_c][0].v  = g_end_logo_v[n];
                g_end_spr_c[page_c][0].w  = g_end_logo_w[n];
                g_end_spr_c[page_c][0].h  = g_end_logo_h[n];
                g_end_spr_c[page_c][0].my = g_end_logo_h[n] / 2;
                g_end_spr_c[page_c][0].cx = 0;
                g_end_spr_c[page_c][0].cy = 0x1E2;
                page_c ^= 1;
            } else {
                g_end_show_sprite = 1;
            }
            g_end_script++;
            break;
        case 9:
            if (fade == 0 && wait == 0 && g_cd_busy == -1) {
                goto done;
            }
            break;
        case 11:
            if (fade == 0 && wait == 0) {
                if (g_end_seq != -1) {
                    SsSeqClose(g_end_seq);
                }
                if (g_end_vab != -1) {
                    SsVabClose(g_end_vab);
                }
                g_end_script++;
                CdIntToPos(g_end_sectors[*g_end_script] + g_end_base, &pos);
                CdReadFileToAddr(&pos, g_end_sectors[*g_end_script + 1] - g_end_sectors[*g_end_script],
                                 (u_long *)0x801C0000);
                g_end_vab = SsVabOpenHead((u_char *)0x801C0004, -1);
                CdIntToPos(g_end_sectors[*g_end_script + 1] + g_end_base, &pos);
                CdReadFileToAddr(&pos, g_end_sectors[*g_end_script + 2] - g_end_sectors[*g_end_script + 1],
                                 (u_long *)0x80140000);
                SsVabTransBody((u_char *)0x80140004, g_end_vab);
                SsVabTransCompleted(SS_WAIT_COMPLETED);
                CdIntToPos(g_end_sectors[*g_end_script + 2] + g_end_base, &pos);
                CdReadFileToAddr(&pos, g_end_sectors[*g_end_script + 3] - g_end_sectors[*g_end_script + 2],
                                 (u_long *)0x801D0000);
                g_end_seq = SsSeqOpen((u_long *)0x801D0004, g_end_vab);
                SsSeqSetVol(g_end_seq, 127, 127);
                SsSeqPlay(g_end_seq, SSPLAY_PLAY, 1);
                g_end_script++;
            }
            break;
        }

        /* An image that has finished streaming in goes to its layer. */
        if (g_cd_busy == -1 && load != 0) {
            switch (load) {
            case 1:
                EndLoadTim((u_long *)0x80180004, 1, 0x140, page_a);
                load = 0;
                if (g_end_image.pw > 256) {
                    EndSetSprite(&g_end_spr_a[page_a][0], 256, 0x82, 0, 0, page_a * 16 + 5);
                    EndSetSprite(&g_end_spr_a[page_a][1], 64, 0x82, 256, 0, page_a * 16 + 9);
                } else {
                    EndSetSprite(&g_end_spr_a[page_a][0], g_end_image.pw, 0x82,
                                 g_end_kind_x[g_end_image_kind], g_end_kind_y[g_end_image_kind],
                                 page_a * 16 + 5);
                    EndSetSprite(&g_end_spr_a[page_a][1], 0, 0x82, 0, 0, 0);
                }
                page_a ^= 1;
                break;
            case 2:
                EndLoadTim((u_long *)0x80180004, 1, 0x280, page_b);
                load = 0;
                EndSetSprite(&g_end_spr_b[page_b], 0x95, 0x82,
                             g_end_kind_x[g_end_image_kind], g_end_kind_y[g_end_image_kind],
                             page_b * 16 + 10);
                page_b ^= 1;
                break;
            case 3:
                EndLoadTim((u_long *)0x80180004, 0, 0x340, page_c);
                load = 0;
                if (g_end_image.pw * 4 > 256) {
                    EndSetSprite(&g_end_spr_c[page_c][0], 256, 0x80, 0, 0, page_c * 16 + 13);
                    EndSetSprite(&g_end_spr_c[page_c][1], 64, 0x80, 256, 0, page_c * 16 + 14);
                } else {
                    EndSetSprite(&g_end_spr_c[page_c][0], g_end_image.pw * 4, 0x80,
                                 g_end_kind_x[g_end_image_kind], g_end_kind_y[g_end_image_kind],
                                 page_c * 16 + 13);
                    EndSetSprite(&g_end_spr_c[page_c][1], 0, 0x80, 0, 0, 0);
                }
                page_c ^= 1;
                break;
            }
        }

        /* The layers fading in or out step their brightness; at the end of
           a fade in the shown page goes opaque, at the end of a fade out it
           is hidden and the other page comes forward. */
        if (fade) {
            level += step * 2;
            if (fade & 1) {
                g_end_spr_a[g_end_page_a][0].r = g_end_spr_a[g_end_page_a][0].g =
                    g_end_spr_a[g_end_page_a][0].b = level;
                g_end_spr_a[g_end_page_a][1].r = g_end_spr_a[g_end_page_a][1].g =
                    g_end_spr_a[g_end_page_a][1].b = level;
            }
            if (fade & 2) {
                g_end_spr_b[g_end_page_b].r = g_end_spr_b[g_end_page_b].g =
                    g_end_spr_b[g_end_page_b].b = level;
            }
            if (fade & 4) {
                g_end_spr_c[g_end_page_c][0].r = g_end_spr_c[g_end_page_c][0].g =
                    g_end_spr_c[g_end_page_c][0].b = level;
                g_end_spr_c[g_end_page_c][1].r = g_end_spr_c[g_end_page_c][1].g =
                    g_end_spr_c[g_end_page_c][1].b = level;
            }
            if (fade & 8) {
                g_end_sprite.r = g_end_sprite.g = g_end_sprite.b = level;
            }
            if (level == 0 || level == 0x80) {
                if (level == 0x80) {
                    if (fade & 1) {
                        g_end_spr_a[g_end_page_a][0].attribute &= 0xBFFFFFFF;
                        g_end_spr_a[g_end_page_a][1].attribute &= 0xBFFFFFFF;
                    }
                    if (fade & 2) {
                        g_end_spr_b[g_end_page_b].attribute &= 0xBFFFFFFF;
                    }
                    if (fade & 4) {
                        g_end_spr_c[g_end_page_c][0].attribute &= 0xBFFFFFFF;
                        g_end_spr_c[g_end_page_c][1].attribute &= 0xBFFFFFFF;
                    }
                } else {
                    if (fade & 1) {
                        g_end_spr_a[g_end_page_a][0].attribute |= 0x80000000;
                        g_end_spr_a[g_end_page_a][1].attribute |= 0x80000000;
                        g_end_page_a ^= 1;
                    }
                    if (fade & 2) {
                        g_end_spr_b[g_end_page_b].attribute |= 0x80000000;
                        g_end_page_b ^= 1;
                    }
                    if (fade & 4) {
                        g_end_spr_c[g_end_page_c][0].attribute |= 0x80000000;
                        g_end_spr_c[g_end_page_c][1].attribute |= 0x80000000;
                        g_end_page_c ^= 1;
                    }
                }
                fade = 0;
            }
        }
        if (wait && !fade) {
            wait--;
        }
        EndDraw();
    }

done:
    switch (ending) {
    case 0:
        MAP_ID = (g_event_flags[0x2D] & 2) ? 0xD8 : 0xD9;
        MAP_ROOM = 0;
        g_movie_next_state = 3;
        break;
    case 3:
        MAP_ID = 0x1CF;
        MAP_ROOM = 0;
        g_movie_next_state = 3;
        break;
    case 4:
        g_movie_next_state = -1;
        break;
    }
    SetDispMask(0);
    DrawSync(0);
    PadStop();
    ResetGraph(0);
    StopCallback();
}
#else
INCLUDE_ASM("end/nonmatchings/end", main);
#endif

/* Brings the display and the sound up for the credits: two ordering tables
   at 320x240, every sprite hidden, the sequencer ready with nothing open. */
void EndInit(void)
{
    RECT rect;
    int  i;

    SetDispMask(0);
    ResetGraph(3);
    GsInitGraph2(320, 240, 4, 0, 0);
    setRECT(&rect, 0, 0, 512, 480);
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    GsDefDispBuff(0, 0, 0, 240);
    g_end_ot[0].length = g_end_ot[1].length = 4;
    g_end_ot[0].org    = g_end_ot_tags[0];
    g_end_ot[1].org    = g_end_ot_tags[1];
    SetDispMask(1);

    for (i = 0; i < 2; i++) {
        EndSetSprite(&g_end_spr_a[i][0], 0, 0x80, 0, 0, 0);
        EndSetSprite(&g_end_spr_a[i][1], 0, 0x80, 0, 0, 0);
        EndSetSprite(&g_end_spr_c[i][0], 0, 0x80, 0, 0, 0);
        EndSetSprite(&g_end_spr_c[i][1], 0, 0x80, 0, 0, 0);
    }
    EndSetSprite(&g_end_spr_b[0], 0, 0x80, 0, 0, 0);
    EndSetSprite(&g_end_spr_b[1], 0, 0x80, 0, 0, 0);

    if (g_options[0]) {
        SsSetStereo();
    } else {
        SsSetMono();
    }
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_C);
    SsUtReverbOn();
    SsSetTickMode(SS_TICK60);
    SsSetTableSize(g_end_seq_table, 1, 1);
    if (g_end_vab != -1) {
        SsVabClose(g_end_vab);
    }
    if (g_end_seq != -1) {
        SsSeqClose(g_end_seq);
    }
    g_end_vab = g_end_seq = -1;
    SsStart();
    SsSetMVol(127, 127);
    VSync(60);
    SsUtSetReverbDepth(64, 64);
}

/* Starts the next credit image streaming in: the script names it, and the
   sector table gives where it starts in END.BIN and how long it runs. */
void EndLoadImage(void)
{
    CdlLOC pos;

    g_end_script++;
    g_end_image_kind = g_end_image_kinds[*g_end_script];
    CdIntToPos(g_end_sectors[*g_end_script] + g_end_base, &pos);
    CdReadFileToAddrAsync(&pos,
                          g_end_sectors[*g_end_script + 1] - g_end_sectors[*g_end_script],
                          (u_long *)0x80180000);
}

/* Uploads the TIM at `tim` to x in the VRAM page `y`, and its CLUT to the
   row under the display unless `noclut`. */
int EndLoadTim(u_long *tim, int noclut, int x, int y)
{
    RECT rect;

    GsGetTimInfo(tim + 1, &g_end_image);
    g_end_image.px = rect.x = x;
    g_end_image.py = rect.y = (y & 1) << 8;
    rect.w = g_end_image.pw;
    rect.h = g_end_image.ph;
    LoadImage(&rect, g_end_image.pixel);
    if (!noclut && (g_end_image.pmode >> 3 & 1)) {
        g_end_image.cx = rect.x = 0;
        g_end_image.cy = rect.y = y + 480;
        rect.w = g_end_image.cw;
        rect.h = g_end_image.ch;
        LoadImage(&rect, g_end_image.clut);
    }
}

/* A sprite of the image last loaded: `w` wide and as high as the image, at
   (x, y) on `tpage`. The low seven bits of `mode` go to the attribute's top
   byte, and bit 7 hides it. */
void EndSetSprite(sp, w, mode, x, y, tpage)
    GsSPRITE *sp;
    u_short   w;
    int       mode;
    int       x;
    int       y;
    int       tpage;
{
    sp->attribute = (mode & 0x7F) << 24 | ((mode & 0x80) ? 0xC0000000 : 0x40000000);
    sp->w       = w;
    sp->h       = g_end_image.ph;
    sp->mx      = w / 2;
    sp->my      = g_end_image.ph / 2;
    sp->tpage   = tpage;
    sp->u       = 0;
    sp->v       = 0;
    sp->cx      = g_end_image.cx;
    sp->cy      = g_end_image.cy;
    sp->r       = 0;
    sp->g       = 0;
    sp->b       = 0;
    sp->rotate  = 0;
    sp->scalex  = 0x1000;
    sp->scaley  = 0x1000;
    sp->x       = x;
    sp->y       = y;
}

/* Draws a frame of the credits and reads the pad. */
void EndDraw(void)
{
    int i;

    g_end_buf = GsGetActiveBuff();
    GsSetWorkBase((PACKET *)(g_end_packets + (g_end_buf << 12)));
    GsClearOt(0, 0, &g_end_ot[g_end_buf]);
    for (i = 0; i < 2; i++) {
        GsSortFastSprite(&g_end_spr_a[g_end_page_a][i], &g_end_ot[g_end_buf], 2);
    }
    GsSortFastSprite(&g_end_spr_b[0], &g_end_ot[g_end_buf], 1);
    GsSortFastSprite(&g_end_spr_b[1], &g_end_ot[g_end_buf], 1);
    for (i = 0; i < 2; i++) {
        GsSortFastSprite(&g_end_spr_c[g_end_page_c][i], &g_end_ot[g_end_buf], 0);
    }
    if (g_end_show_sprite) {
        GsSortFastSprite(&g_end_sprite, &g_end_ot[g_end_buf], 0);
    }
    g_end_pad_old  = g_end_pad;
    g_end_pad      = PadRead(1);
    g_end_pad_trig = (g_end_pad & g_end_pad_old) ^ g_end_pad;
    DrawSync(0);
    VSync(2);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &g_end_ot[g_end_buf]);
    GsDrawOt(&g_end_ot[g_end_buf]);
}

#ifdef NON_MATCHING
/* Plays g_end_movie. 0 when it ran to its end, 1 when the stream did. */
int EndPlayMovie(void)
{
    CdlFILE file;
    RECT    rect;
    char    name[] = "\\STR0\\MV00.STR;1";
    u_char  unused[0x74];

    SetDispMask(0);
    CdControlB(CdlPause, 0, 0);
    SsEnd();
    SsInit();
    if (g_options[0]) {
        SsSetStereo();
    } else {
        SsSetMono();
    }
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetSerialVol(SS_SERIAL_A, 0x7FFF, 0x7FFF);
    g_end_pad = -1;
    setRECT(&rect, 0, 0, 511, 479);
    ClearImage(&rect, 0, 0, 0);
    DecDCTReset(0);
    DecDCToutCallback(StrOutCallback);
    StSetRing(g_ring_buf, 20);
    StSetStream(1, 1, -1, 0, 0);
    StrSetDecEnv(&g_dec, 0, 0, 0, 240);
    GsInitGraph(320, 240, 4, 0, 1);
    GsDefDispBuff(g_dec.rect[0].x, g_dec.rect[0].y, g_dec.rect[1].x, g_dec.rect[1].y);
    SetDispMask(1);
    g_dec.vlc_id  = 0;
    g_dec.rect_id = 0;
    g_dec.done    = 0;
    StrPutHex(g_end_movie, &name[9], 2);
    StrPutHex(g_end_movie / 16, &name[4], 1);
    while (!CdSearchFile(&file, name)) {
    }
    g_end_movie_frame = 0;
    g_end_movie_done  = 0;
    g_str_loc.minute = file.pos.minute;
    g_str_loc.second = file.pos.second;
    g_str_loc.sector = file.pos.sector;
    StrKickCD(&g_str_loc);
    StrDecodeNextFrame(&g_dec);
    for (;;) {
        DecDCTin(g_dec.vlc_buf[g_dec.vlc_id], 1);
        DecDCTout(g_dec.img_buf, g_dec.slice.w * g_dec.slice.h / 2);
        StrDecodeNextFrame(&g_dec);
        g_end_movie_frame++;
        StrSync(&g_dec);
        g_end_pad_old  = g_end_pad;
        g_end_pad      = PadRead(1);
        g_end_pad_trig = (g_end_pad & g_end_pad_old) ^ g_end_pad;
        VSync(0);
        ResetGraph(1);
        GsSwapDispBuff();
        if (g_end_movie_frame >= g_end_movie_frames[g_end_movie] - 5) {
            return 0;
        }
        if (g_end_movie_done == 1) {
            return 1;
        }
    }
}
#else
INCLUDE_ASM("end/nonmatchings/end", EndPlayMovie);
#endif

/* The MDEC's end-of-slice callback: loads the slice just decoded, then
   either starts the next one or, at the end of the area, flips the areas
   and flags the frame done. */
void StrOutCallback(void)
{
    if (StCdIntrFlag) {
        StCdInterrupt();
        StCdIntrFlag = 0;
    }
    LoadImage(&g_dec.slice, g_dec.img_buf);
    g_dec.slice.x += g_dec.slice.w;
    if (g_dec.slice.x < g_dec.rect[g_dec.rect_id].x + g_dec.rect[g_dec.rect_id].w) {
        DrawSync(0);
        DecDCTout(g_dec.img_buf, g_dec.slice.w * g_dec.slice.h / 2);
    } else {
        g_dec.done    = 1;
        g_dec.rect_id = (g_dec.rect_id == 0);
        g_dec.slice.x = g_dec.rect[g_dec.rect_id].x;
        g_dec.slice.y = g_dec.rect[g_dec.rect_id].y;
    }
}
