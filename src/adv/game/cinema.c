/* Persona 1 (JP) - the cinema frame.  ADV only.
 *   0x800AEE9C CinemaOpen    0x800AF1D8 CinemaClose
 *
 * Script commands 60 and 61 put a frame round the screen for a cut scene and
 * take it away. The frame is one slot whose definition is a list of eight
 * parts, each a cel list drawn at an offset. Opening it widens the side
 * pieces from the middle while the room behind darkens (CinemaDim), then
 * slides the top and bottom pieces in, growing the bars as they come;
 * closing runs the same steps backwards. `plain` says who pumps the frames:
 * 0 while the field is running, anything else when only the display is.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/bg.h>
#include <persona/common/slot.h>

#define CINEMA_SLOT  0x2F
#define CINEMA_LAYER 0x10   /* the g_bg_shown bit the message layer takes */

/* A slot definition that draws several pieces at once. */
typedef struct {
    /* 0x00 */ void  *cels;
    /* 0x04 */ short  x, y;
    /* 0x08 */ int    flag;
} SlotPart;                     /* 0xC bytes, closed by -1 */

/* The head of a cel list; the cels follow it, eight bytes each. */
typedef struct {
    /* 0x00 */ u_char  count;
    /* 0x01 */ u_char  unk1;
    /* 0x02 */ u_short attr;
    /* 0x04 */ u_short w, h;
} CelHead;

typedef struct {
    /* 0x00 */ CelHead head;
    /* 0x08 */ u_char  u, v;
    /* 0x0A */ u_char  pad0A[6];
} CelList1;

extern SlotPart g_cinema_parts[];
extern CelHead  g_cinema_cels0, g_cinema_cels1, g_cinema_cels3,
                g_cinema_cels4, g_cinema_cels5, g_cinema_cels6;
extern CelList1 g_cinema_cels2, g_cinema_cels_spare;
extern int      g_actor_dim;

extern void SlotClear(u_char slot);
extern void SlotsApplyXScale(void);
extern void ActorsSetDepth();
extern void ActorsPlaceSprites(void);
extern void AdvRunFrame(void);
extern void RunFrame(void);
extern void CinemaDim(short step);

#define P g_cinema_parts

#define FRAME(plain)       \
    if ((plain) == 0) {    \
        AdvRunFrame();     \
    } else {               \
        RunFrame();        \
    }

/* 97.80%: the original moves the cels0 bar's address out of the second loop
   and this does not. loop.c keeps an invariant in the loop unless its
   lifetime times the threshold covers the loop's insn count; here the
   address lives three insns in a 53-insn loop and falls just short, where
   CinemaClose's first loop (lifetime four, the -8 constant between) makes
   it. No reordering, cast or loop shape tried changes either count. */
#ifdef NON_MATCHING
void CinemaOpen(short plain)
{
    int   step;
    short y;

    step = 0;
    g_actor_dim = 1;
    g_bg_layers[4].r = 0x80;
    g_bg_layers[4].g = 0x80;
    g_bg_layers[4].b = 0x80;
    SlotInitTagged(g_cinema_parts, CINEMA_SLOT, 0x1F, 0x10, 0x98);
    if (plain == 0) {
        SlotsApplyXScale();
        ActorsSetDepth(0, 0xF);
        ActorsPlaceSprites();
    }
    P[0].y = 0x24;
    P[1].y = 0x1C;
    P[2].y = 0x1C;
    P[3].y = 0x24;
    P[4].y = 0x24;
    P[5].y = 0x24;
    P[6].y = 0x24;
    P[1].x = 0x40;
    P[3].x = 0x40;
    P[2].x = 0xB8;
    P[7].x = 0xB8;
    g_cinema_cels0.h = 0;
    g_cinema_cels4.h = 0;
    g_cinema_cels5.h = 0;
    g_cinema_cels6.h = 0;
    g_cinema_cels1.w = 0x78;
    g_cinema_cels2.head.w = 0x28;
    g_cinema_cels3.w = 0x78;
    g_cinema_cels_spare.head.w = 0x28;
    g_cinema_cels_spare.u = 0x40;
    do {
        FRAME(plain);
        P[1].x -= 8;
        P[3].x -= 8;
        g_cinema_cels1.w += 0x10;
        g_cinema_cels3.w += 0x10;
        P[2].x += 8;
        P[7].x += 8;
        CinemaDim(step);
        step++;
    } while (g_cinema_cels1.w != 0xF8);
    y = P[0].y;
    while (y != 8) {
        FRAME(plain);
        y = (P[0].y -= 4);
        P[1].y -= 4;
        P[2].y -= 4;
        P[3].y += 4;
        P[4].y -= 4;
        P[5].y -= 4;
        P[6].y -= 4;
        P[7].y += 4;
        g_cinema_cels0.h += 8;
        g_cinema_cels6.h += 8;
        g_cinema_cels4.h += 8;
        g_cinema_cels5.h += 8;
    }
    g_bg_shown |= CINEMA_LAYER;
}
#else
INCLUDE_ASM("adv/nonmatchings/game/cinema", CinemaOpen);
#endif

void CinemaClose(short plain)
{
    int   step;
    short y;

    step = 7;
    if (plain == 0) {
        SlotsApplyXScale();
        ActorsSetDepth(0, 0xF);
        ActorsPlaceSprites();
    }
    g_bg_shown ^= CINEMA_LAYER;
    y = P[0].y;
    while (y != 0x24) {
        FRAME(plain);
        y = (P[0].y += 4);
        P[1].y += 4;
        P[2].y += 4;
        P[3].y -= 4;
        P[4].y += 4;
        P[5].y += 4;
        P[6].y += 4;
        P[7].y -= 4;
        g_cinema_cels0.h -= 8;
        g_cinema_cels4.h -= 8;
        g_cinema_cels5.h -= 8;
        g_cinema_cels6.h -= 8;
    }
    while (g_cinema_cels1.w != 0x78) {
        FRAME(plain);
        P[1].x += 8;
        P[3].x += 8;
        g_cinema_cels1.w -= 0x10;
        g_cinema_cels3.w -= 0x10;
        P[2].x -= 8;
        P[7].x -= 8;
        CinemaDim(step);
        step--;
    }
    FRAME(plain);
    SlotClear(CINEMA_SLOT);
    g_actor_dim = 0;
}
