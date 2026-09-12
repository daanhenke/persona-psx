/* Persona 1 (JP) - the portrait beside a battle message.  BTLP only.
 *   0x800761B0 BtlFaceLoad
 *
 * Reads one character's portrait off the disc, uploads it, and builds the
 * three primitives that show it: the big quad, a small sprite of the same
 * artwork, and the draw mode that selects its page. Each is built once and
 * copied onto its second copy, because the battle keeps a pair of everything -
 * one per frame buffer.
 *
 * A portrait already loaded is not read again unless the caller insists, which
 * is what lets the negotiation put the same face up between scenes without
 * spinning the drive.
 *
 * BtlFaceLoadFile behind it is the same routine with the test taken out and
 * the file named outright rather than looked up by character, which is what
 * the scene player and the opening dialogue want: they know which portrait
 * file they need and there is no character to key the cache on. It leaves the
 * cache saying nothing is loaded, so the next BtlFaceLoad reads again.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>

/* Where the file is read to, and where its TIM starts inside it. */
#define FACE_STAGE ((u_long *)0x80140000)
#define FACE_TIM   ((u_long *)0x80140008)

/* The scene kind the portraits are filed under. */
#define FACE_SCENE_KIND 3

/* Where the artwork lands in VRAM: the CLUT on the strip at the bottom of the
   frame buffers, the pixels in the page beside them. */
#define FACE_CLUT_X 0
#define FACE_CLUT_Y 0x1F2
#define FACE_CLUT_W 0x100
#define FACE_CLUT_H 1
#define FACE_TEX_X  0x350
#define FACE_TEX_Y  0x120
#define FACE_TEX_W  0x2C
#define FACE_TEX_H  0x60

/* The portrait's corner of that page. */
#define FACE_U0 0x20
#define FACE_V0 0x20
#define FACE_U1 0x78
#define FACE_V1 0x80

extern u_char   g_btl_face_files[];
extern POLY_FT4 g_btl_face_prim[];
extern SPRT     g_btl_face_sprt[];
extern DR_MODE  g_btl_face_mode[];
extern int      g_btl_face_step;

extern CdlFILE  g_adv_scene_file;
extern int      g_cd_busy;

/* The index is `short` where the routine is defined, and every other caller
   declares it that way; this unit does not. BtlFaceLoadFile hands it an int
   and the image passes the register straight through, so the declaration this
   translation unit was built against had no narrowing in it. */
extern void AdvResolveSceneLoc(short kind, int index, void *unused);
extern void CdReadFileToAddrAsync(CdlFILE *file, int sectors, u_long *dest);

void BtlFaceLoad(int who, int always)
{
    TIM_IMAGE tim;

    if (always != 0 || g_btl_face_id != who) {
        AdvResolveSceneLoc(FACE_SCENE_KIND, g_btl_face_files[who], 0);
        CdReadFileToAddrAsync(&g_adv_scene_file, g_adv_scene_file.size,
                              FACE_STAGE);
        while (g_cd_busy != -1) {
            BtlDrawFrame();
        }
        OpenTIM(FACE_TIM);
        ReadTIM(&tim);
        BtlQueueVramLoad(tim.caddr, FACE_CLUT_X, FACE_CLUT_Y,
                         FACE_CLUT_W, FACE_CLUT_H);
        BtlQueueVramLoad(tim.paddr, FACE_TEX_X, FACE_TEX_Y,
                         FACE_TEX_W, FACE_TEX_H);
        g_btl_face_id = who;
    }

    SetPolyFT4(g_btl_face_prim);
    SetSemiTrans(g_btl_face_prim, 0);
    SetShadeTex(g_btl_face_prim, 1);
    g_btl_face_prim[0].u0 = FACE_U0;
    g_btl_face_prim[0].v0 = FACE_V0;
    g_btl_face_prim[0].u1 = FACE_U1;
    g_btl_face_prim[0].v1 = FACE_V0;
    g_btl_face_prim[0].u2 = FACE_U0;
    g_btl_face_prim[0].v2 = FACE_V1;
    g_btl_face_prim[0].u3 = FACE_U1;
    g_btl_face_prim[0].v3 = FACE_V1;
    g_btl_face_prim[0].tpage = GetTPage(1, 0, FACE_TEX_X, FACE_TEX_Y);
    g_btl_face_prim[0].clut = GetClut(FACE_CLUT_X, FACE_CLUT_Y);

    SetSprt(g_btl_face_sprt);
    SetSemiTrans(g_btl_face_sprt, 0);
    SetShadeTex(g_btl_face_sprt, 1);
    g_btl_face_sprt[0].w = FACE_U1 - FACE_U0;
    g_btl_face_sprt[0].u0 = FACE_U0;
    g_btl_face_sprt[0].v0 = FACE_V0;
    g_btl_face_sprt[0].h = FACE_V1 - FACE_V0;
    g_btl_face_sprt[0].clut = GetClut(FACE_CLUT_X, FACE_CLUT_Y);

    SetDrawMode(g_btl_face_mode, 0, 0,
                GetTPage(1, 0, FACE_TEX_X, FACE_TEX_Y), 0);

    g_btl_face_prim[1] = g_btl_face_prim[0];
    g_btl_face_sprt[1] = g_btl_face_sprt[0];
    g_btl_face_mode[1] = g_btl_face_mode[0];
    g_btl_face_step = 0;
}

void BtlFaceLoadFile(int file)
{
    TIM_IMAGE tim;

    AdvResolveSceneLoc(FACE_SCENE_KIND, file, 0);
    CdReadFileToAddrAsync(&g_adv_scene_file, g_adv_scene_file.size,
                          FACE_STAGE);
    while (g_cd_busy != -1) {
        BtlDrawFrame();
    }
    OpenTIM(FACE_TIM);
    ReadTIM(&tim);
    BtlQueueVramLoad(tim.caddr, FACE_CLUT_X, FACE_CLUT_Y,
                     FACE_CLUT_W, FACE_CLUT_H);
    BtlQueueVramLoad(tim.paddr, FACE_TEX_X, FACE_TEX_Y,
                     FACE_TEX_W, FACE_TEX_H);
    g_btl_face_id = -1;

    SetPolyFT4(g_btl_face_prim);
    SetSemiTrans(g_btl_face_prim, 0);
    SetShadeTex(g_btl_face_prim, 1);
    g_btl_face_prim[0].u0 = FACE_U0;
    g_btl_face_prim[0].v0 = FACE_V0;
    g_btl_face_prim[0].u1 = FACE_U1;
    g_btl_face_prim[0].v1 = FACE_V0;
    g_btl_face_prim[0].u2 = FACE_U0;
    g_btl_face_prim[0].v2 = FACE_V1;
    g_btl_face_prim[0].u3 = FACE_U1;
    g_btl_face_prim[0].v3 = FACE_V1;
    g_btl_face_prim[0].tpage = GetTPage(1, 0, FACE_TEX_X, FACE_TEX_Y);
    g_btl_face_prim[0].clut = GetClut(FACE_CLUT_X, FACE_CLUT_Y);

    SetSprt(g_btl_face_sprt);
    SetSemiTrans(g_btl_face_sprt, 0);
    SetShadeTex(g_btl_face_sprt, 1);
    g_btl_face_sprt[0].w = FACE_U1 - FACE_U0;
    g_btl_face_sprt[0].u0 = FACE_U0;
    g_btl_face_sprt[0].v0 = FACE_V0;
    g_btl_face_sprt[0].h = FACE_V1 - FACE_V0;
    g_btl_face_sprt[0].clut = GetClut(FACE_CLUT_X, FACE_CLUT_Y);

    SetDrawMode(g_btl_face_mode, 0, 0,
                GetTPage(1, 0, FACE_TEX_X, FACE_TEX_Y), 0);

    g_btl_face_prim[1] = g_btl_face_prim[0];
    g_btl_face_sprt[1] = g_btl_face_sprt[0];
    g_btl_face_mode[1] = g_btl_face_mode[0];
    g_btl_face_step = 0;
}
