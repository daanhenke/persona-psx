/* Persona 1 (JP) - the field's party and its frame.  S2D.
 *   0x8008E8B4 S2dDrawParty      0x8008EC44 S2dBindCompass
 *   0x8008ED0C S2dCompassMoveTo  0x8008ED54 S2dCompassScaleTo
 *   0x8008EDB4 S2dCompassStep    0x8008EFB8 S2dCompassDepthSort
 *   0x8008F1BC S2dDrawCompass    0x8008F3C0 S2dUpdate
 *   0x8008F4A4 S2dDraw           0x8008F570 S2dInitPartyObjs
 *   0x8008F7D8 S2dFilterInput    0x8008F878 S2dViewKeys
 *   0x8008FC00 S2dPartyMoved
 *
 * The party is two placed objects, g_party_obj and [1]. Left standing
 * still long enough on a map that allows it, one of them starts an idle
 * animation picked at random: [1] wobbles, or (one time in 256) [0]
 * breathes. Moving again puts both back.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/s2d/s2d.h>
#include <persona/s2d/model.h>

/* Frames the party has to stand still before an idle animation starts. */
#define IDLE_FRAMES 9000

/* The idle animation: none picked yet, the wobble, the breath. */
#define IDLE_PICK   0
#define IDLE_WOBBLE 1
#define IDLE_BREATH 2

extern int   D_800B0E84[];
extern int   D_800A4CF0;
extern int   g_idle_frames;
extern int   g_idle_kind;
extern int   g_idle_phase;
/* How far the party's model turns each frame. */
extern int   g_party_spin;

extern short g_compass_x;

/* The compass's slide and scale: from where it was to the target, by t
   going up by the step to 0x1000. */
typedef struct {
    /* 0x00 */ short x;
    /* 0x02 */ short y;
    /* 0x04 */ int   step;
    /* 0x08 */ int   scale_step;
    /* 0x0C */ u_char order[4];  /* the four outer pieces, back to front */
} S2dCompassMove;

extern short          g_compass_from[2];
extern S2dCompassMove g_compass_move;
extern int            g_compass_t;
extern VECTOR         g_compass_scale_from;
extern VECTOR         g_compass_scale_to;
extern int            g_compass_scale_t;

extern void func_80099A00(void);
extern void func_80099AD8(GsOT *ot, int n);

extern u_char D_800B1D38[];
extern int    D_800B863C;
extern short  g_compass_y;

extern void func_8009777C(u_char *p);
extern void func_80091270(void);
extern void func_8008CEB8(void);
extern void func_8008BD78(void);
extern void func_8008C038(void);
extern void func_8008C34C(void);
extern void func_8008AEA8(void);
extern void func_8008BA9C(void);
extern void func_80093784(void);
extern void S2dBeginFrame(int alt);
extern void func_8009929C(int a, GsOT *ot, int b, int x, int y, int c, int d);
extern void S2dDrawMapObjs(void);
extern void func_800937FC(void);
extern void func_800971D4(void);
extern void func_80089F5C(int n);
extern void S2dLoad2670(void);

/* S2dFilterInput's two inputs, their last sixteen values, the outputs and
   the sixteen weights. */
extern u_char D_800B2D98[2];
extern short  g_filter_hist[2][16];
extern int    g_filter_sum[2];
extern short  D_800A5028[16];

#define g_seq_handle ((short *)0x801F537C)

/* The view's zoom: whether it was last zoomed out, and a request to put it
   back. */
extern int D_800A5068;
/* Which way the view was last zoomed: 2 back in, below that out. */
extern int g_view_zoom;

extern int  func_8008E158(int which);
extern void func_800999D0(int a, int b);

/* The party's last six places. */
extern short D_800A5088[][2];

/* The four outer pieces' places, projected for their depths. */
extern SVECTOR g_compass_pts[4];
extern long    g_compass_sz[4];

extern void func_8008D988(VECTOR *trans, SVECTOR *rot, VECTOR *scale,
                          GsCOORDINATE2 *coord);
extern void func_8009994C(int x, int y, int w, int h, int a, int b, int c);

void S2dDrawParty(GsOT *ot, int shift)
{
    MATRIX     ls;
    MATRIX     lw;
    SVECTOR    rot;
    S2dMapObj *o;

    GsSetRefView2(&g_s2d_view);
    if (D_800B0E84[8] == 0 && D_800A4CF0 == 1) {
        g_idle_frames++;
        if (g_idle_frames > IDLE_FRAMES) {
            g_idle_frames = IDLE_FRAMES;
        }
        if (g_idle_frames == IDLE_FRAMES) {
            switch (g_idle_kind) {
            case IDLE_PICK:
                if (!(rand() & 0xFF)) {
                    g_idle_kind = IDLE_BREATH;
                } else {
                    g_idle_kind = IDLE_WOBBLE;
                }
                break;
            case IDLE_WOBBLE:
                g_idle_phase++;
                g_party_mark.scale.vz =
                    (((rsin(g_idle_phase * 80 + 0xA00) + ONE) * 0x400) >> 12) + ONE;
                break;
            case IDLE_BREATH:
                g_idle_phase++;
                g_party_obj.scale.vx =
                    ((((rsin(g_idle_phase * 80 + 0xC00) + ONE) >> 1) * 0xC00) >> 12) + 0x1800;
                g_party_obj.scale.vz = g_party_obj.scale.vx;
                g_party_obj.scale.vy = g_party_obj.scale.vx;
                break;
            }
        }
    } else {
        g_party_mark.scale.vz = ONE;
        g_party_mark.rot.vz = 0;
        g_party_obj.scale.vx = 0x1800;
        g_party_obj.scale.vy = 0x1800;
        g_party_obj.scale.vz = 0x1800;
        g_idle_frames = 0;
        g_idle_kind = IDLE_PICK;
        g_idle_phase = 0;
    }

    o = &g_party_obj;
    g_party_obj.rot.vy += g_party_spin;
    func_8008D988(&o->trans, &o->rot, &o->scale, &o->coord);
    GsGetLws(g_party_obj.obj.coord2, &lw, &ls);
    GsSetLightMatrix(&lw);
    GsSetLsMatrix(&ls);
    GsSortObject4(&o->obj, ot, shift, getScratchAddr(0));

    o = &g_party_mark;
    rot = g_party_mark.rot;
    func_8008D988(&o->trans, &rot, &o->scale, &o->coord);
    GsGetLws(g_party_mark.obj.coord2, &lw, &ls);
    GsSetLightMatrix(&lw);
    GsSetLsMatrix(&ls);
    GsSortObject4(&o->obj, ot, shift, getScratchAddr(0));

    o = &D_800B0F1C;
    if (o->kind != 0) {
        func_8008D988(&o->trans, &o->rot, &o->scale, &o->coord);
        GsGetLws(D_800B0F1C.obj.coord2, &lw, &ls);
        GsSetLightMatrix(&lw);
        GsSetLsMatrix(&ls);
        GsSortObject4(&o->obj, &g_ot_back[g_draw_side], 10, getScratchAddr(0));
    }
}

/* The compass's five pieces are objects 1 to 5 of model 0 (seven words an
   object). The record is taken by index each turn and the coordinate
   system's address by index again: that is what gives the image its three
   stepped values. */
void S2dBindCompass(void)
{
    S2dMapObj *o;
    int        i;

    for (i = 0; i < 5; i++) {
        o = &g_compass_objs[i];
        o->obj.tmd = &g_models[0].objs[i * 7 + 7];
        *(u_long *)o->obj.tmd[4] &= 0xFF000000;
        *(u_long *)o->obj.tmd[4] |= o->obj.tmd[5];
        o->obj.coord2 = &g_compass_objs[i].coord;
        o->obj.attribute = 0;
    }
    func_8009994C(g_compass_x + 0x10F, 0x18, 0x98, 0x68, 0, 0, 0x1000);
}

void S2dCompassMoveTo(int x, int y, int step)
{
    g_compass_move.x = x;
    g_compass_move.y = y;
    g_compass_t = 0;
    g_compass_move.step = step;
    g_compass_from[0] = g_compass_objs[0].trans.vx;
    g_compass_from[1] = g_compass_objs[0].trans.vy;
}

void S2dCompassScaleTo(int x, int y, int z, int step)
{
    g_compass_scale_to.vx = x;
    g_compass_scale_to.vy = y;
    g_compass_scale_to.vz = z;
    g_compass_scale_t = 0;
    g_compass_move.scale_step = step;
    g_compass_scale_from.vx = g_compass_objs[0].scale.vx;
    g_compass_scale_from.vy = g_compass_objs[0].scale.vy;
    g_compass_scale_from.vz = g_compass_objs[0].scale.vz;
}

/* One frame of the slide and the scale; with turn set, the compass also
   takes the camera's rotation turned by the map's, and the four pieces
   hung off it are turned back so they stay facing the screen. */
void S2dCompassStep(int turn)
{
    int i;

    g_compass_t += g_compass_move.step;
    if (g_compass_t > ONE) {
        g_compass_t = ONE;
    }
    g_compass_objs[0].trans.vx = g_compass_from[0] +
        (((g_compass_move.x - g_compass_from[0]) * g_compass_t) >> 12);
    g_compass_objs[0].trans.vy = g_compass_from[1] +
        (((g_compass_move.y - g_compass_from[1]) * g_compass_t) >> 12);

    g_compass_scale_t += g_compass_move.scale_step;
    if (g_compass_scale_t > ONE) {
        g_compass_scale_t = ONE;
    }
    g_compass_objs[0].scale.vx = g_compass_scale_from.vx +
        (((g_compass_scale_to.vx - g_compass_scale_from.vx) * g_compass_scale_t) >> 12);
    g_compass_objs[0].scale.vy = g_compass_scale_from.vy +
        (((g_compass_scale_to.vy - g_compass_scale_from.vy) * g_compass_scale_t) >> 12);
    g_compass_objs[0].scale.vz = g_compass_scale_from.vz +
        (((g_compass_scale_to.vz - g_compass_scale_from.vz) * g_compass_scale_t) >> 12);

    if (turn) {
        /* The camera's three rotation shorts are one SVECTOR here. */
        g_compass_objs[0].rot = *(SVECTOR *)&g_s2d_cam_rx;
        g_compass_objs[0].rot.vy += g_map_xform.rot.vy;
        for (i = 1; i < 5; i++) {
            g_compass_objs[i].rot.vy = ONE - g_compass_objs[0].rot.vy;
        }
    }
    func_80099A00();
}

/* Orders the four outer pieces back to front: their places are projected
   for the GTE's depths, and the order bubble-sorted on them. The two loop
   counters are what RotTransPers4 hands its p and flag back in. */
void S2dCompassDepthSort(void)
{
    int  i;
    int  j;
    long t;

    for (i = 0; i < 4; i++) {
        g_compass_pts[i].vx = g_compass_pos[i + 1].vx << 8;
        g_compass_pts[i].vy = g_compass_pos[i + 1].vy << 8;
        g_compass_pts[i].vz = g_compass_pos[i + 1].vz << 8;
        g_compass_pts[i].pad = 0;
        g_compass_move.order[i] = i + 1;
    }
    RotTransPers4(&g_compass_pts[0], &g_compass_pts[1], &g_compass_pts[2],
                  &g_compass_pts[3], &g_compass_sz[0], &g_compass_sz[1],
                  &g_compass_sz[2], &g_compass_sz[3], &i, &j);
    ReadSZfifo4(&g_compass_sz[0], &g_compass_sz[1], &g_compass_sz[2],
                &g_compass_sz[3]);
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3 - i; j++) {
            if (g_compass_sz[j] < g_compass_sz[j + 1]) {
                t = g_compass_sz[j];
                g_compass_sz[j] = g_compass_sz[j + 1];
                g_compass_sz[j + 1] = t;
                t = g_compass_move.order[j];
                g_compass_move.order[j] = g_compass_move.order[j + 1];
                g_compass_move.order[j + 1] = t;
            }
        }
    }
}

/* The compass's centre goes behind everything; the four pieces round it,
   back to front, into the layers. */
void S2dDrawCompass(void)
{
    MATRIX m;
    MATRIX unused;  /* the frame has room for a second matrix */
    GsOT  *ots[4] = {
        &g_ot_layer[0][g_draw_side], &g_ot_layer[1][g_draw_side],
        &g_ot_layer[1][g_draw_side], &g_ot_layer[2][g_draw_side],
    };
    int    i;
    int    k;

    GsSetRefView2(&g_s2d_view2);
    func_8008D988(&g_compass_objs[0].trans, &g_compass_objs[0].rot,
                  &g_compass_objs[0].scale, &g_compass_objs[0].coord);
    GsGetLw(g_compass_objs[0].obj.coord2, &m);
    GsSetLightMatrix(&m);
    GsGetLs(g_compass_objs[0].obj.coord2, &m);
    GsSetLsMatrix(&m);
    GsSortObject4(&g_compass_objs[0].obj, &g_ot_back[g_draw_side], 10,
                  getScratchAddr(0));
    S2dCompassDepthSort();
    for (i = 0; i < 4; i++) {
        k = g_compass_move.order[i];
        func_8008D988(&g_compass_objs[k].trans, &g_compass_objs[k].rot,
                      &g_compass_objs[k].scale, &g_compass_objs[k].coord);
        GsGetLw(g_compass_objs[k].obj.coord2, &m);
        GsSetLightMatrix(&m);
        GsGetLs(g_compass_objs[k].obj.coord2, &m);
        GsSetLsMatrix(&m);
        GsSortObject4(&g_compass_objs[k].obj, ots[i], 7, getScratchAddr(0));
    }
    func_80099AD8(&g_ot_back[g_draw_side], 1);
}

/* The frame's update: the map's palette effect, then the field's mode. */
void S2dUpdate(void)
{
    if (g_map_info[g_btl_map_id].effect == 0) {
        func_8009777C(D_800B1D38);
    }
    func_80091270();
    switch (D_800A4CFC.mode) {
    case 0:
    case 1:
        func_8008CEB8();
        break;
    case 2:
        func_8008BD78();
        break;
    case 3:
        func_8008C038();
        break;
    case 4:
        func_8008C34C();
        break;
    }
    func_8008AEA8();
    func_8008BA9C();
    S2dCompassStep(1);
    func_80093784();
}

/* The frame's drawing, back to front. */
void S2dDraw(void)
{
    S2dBeginFrame(0);
    func_8009929C(D_800B863C, &g_ot_back[g_draw_side], 0, g_compass_x,
                  g_compass_y, *(u_char *)0x801F2B30, g_s2d_facing);
    S2dDrawCompass();
    S2dDrawParty(&g_ot_map[g_draw_side], 3);
    S2dDrawMapObjs();
    func_800937FC();
    func_800971D4();
    func_80089F5C(4);
}

/* The party's model at the camera's place, and the three objects that hang
   off D_800B0B78: two of model 4's objects at the camera's rotation. */
void S2dInitPartyObjs(void)
{
    GsInitCoordinate2(NULL, &g_party_obj.coord);
    g_party_obj.trans = *(VECTOR *)&g_s2d_cam_x;

    GsInitCoordinate2(NULL, &D_800B0B78.coord);
    D_800B0B78.scale.vx = ONE;
    D_800B0B78.scale.vy = ONE;
    D_800B0B78.scale.vz = ONE;

    GsInitCoordinate2(&D_800B0B78.coord, &D_800B0C04.coord);
    D_800B0C04.trans.vx = 0;
    D_800B0C04.trans.vy = 0;
    D_800B0C04.trans.vz = ONE;
    D_800B0C04.rot = *(SVECTOR *)&g_s2d_cam_rx;
    D_800B0C04.scale.vx = 0;
    D_800B0C04.scale.vy = 4;
    D_800B0C04.scale.vz = 0;

    GsInitCoordinate2(&D_800B0B78.coord, &D_800B0C90.coord);
    D_800B0C90.trans.vx = 0;
    D_800B0C90.trans.vy = 0;
    D_800B0C90.trans.vz = ONE;
    D_800B0C90.rot = *(SVECTOR *)&g_s2d_cam_rx;
    D_800B0C90.scale.vx = 0;
    D_800B0C90.scale.vy = 0;
    D_800B0C90.scale.vz = 0;

    S2dLoad2670();

    D_800B0C04.obj.tmd = g_models[4].objs;
    *(u_long *)D_800B0C04.obj.tmd[4] &= 0xFF000000;
    *(u_long *)D_800B0C04.obj.tmd[4] |= D_800B0C04.obj.tmd[5];
    D_800B0C04.obj.attribute = 0;
    D_800B0C04.obj.coord2 = &D_800B0C04.coord;

    D_800B0C90.obj.tmd = g_models[4].objs + 7;
    *(u_long *)D_800B0C90.obj.tmd[4] &= 0xFF000000;
    *(u_long *)D_800B0C90.obj.tmd[4] |= D_800B0C90.obj.tmd[5];
    D_800B0C04.obj.attribute = 0x40000000;
    D_800B0C90.obj.attribute = 0;
    D_800B0C90.obj.coord2 = &D_800B0C90.coord;

    g_party_spin = 0x20;
}

/* Two sixteen-tap filters: input n goes into each one's ring, and the
   weighted sum of the ring comes out. */
void S2dFilterInput(int n)
{
    int i;
    int k;

    for (i = 0; i < 2; i++) {
        g_filter_hist[i][n & 0xF] = D_800B2D98[i];
        g_filter_sum[i] = 0;
        for (k = 0; k < 16; k++) {
            g_filter_sum[i] += g_filter_hist[i][(n - k) & 0xF] * D_800A5028[k];
        }
    }
}

/* The view's keys: one zooms the compass out over the map and brings in
   its music, another (or a request) zooms it back; up and down tilt the
   view, and two keys - through the filters - turn it. Always 1. */
int S2dViewKeys(void)
{
    short *p;

    if (func_8008E158(1) & g_s2d_keys[2].mask) {
        S2dCompassMoveTo(g_s2d_heading ? -0x9C : 0x9C, -0x6C, 0x100);
        S2dCompassScaleTo(0xE00, 0xE00, 0xE00, 0x100);
        func_800999D0(0, 0x100);
        func_8009994C(g_s2d_heading == 0 ? 0x1A0 : 0x60, 0x2C, 0x80, 0x38, 1,
                      1, 0x100);
        SsSeqSetVol(g_seq_handle[13], 0x7E, 0x7E);
        SsSeqPlay(g_seq_handle[13], 1, 1);
        SsSeqStop(g_seq_handle[15]);
        if ((u_int)g_view_zoom < 2) {
            D_800A5068 = 1;
        } else {
            g_view_zoom = 0;
            D_800A5068 = 0;
        }
    }
    if ((func_8008E158(1) & (g_s2d_keys[0].mask | 0xF0)) || D_800A5068 == 1) {
        p = &g_compass_x;
        S2dCompassMoveTo(*p + 0x58, -0x18, 0x100);
        S2dCompassScaleTo(ONE, ONE, ONE, 0x100);
        func_8009994C(*p + 0x10F, 0x18, 0x98, 0x68, 0, 0, 0x100);
        func_800999D0(1, 0x100);
        SsSeqSetVol(g_seq_handle[15], 0x7E, 0x7E);
        SsSeqPlay(g_seq_handle[15], 1, 1);
        SsSeqStop(g_seq_handle[13]);
        SsSeqStop(g_seq_handle[14]);
        g_view_zoom = 2;
        D_800A5068 = 0;
    }
    if (func_8008E158(0) & 0x1000) {
        int v;

        v = D_800B0C04.rot.vx - 0xB;
        if (v <= 0) {
            v = 1;
        }
        D_800B0C04.rot.vx = v;
    }
    if (func_8008E158(0) & 0x4000) {
        int v;

        v = D_800B0C04.rot.vx + 0xB;
        if (v > 0x400) {
            v = 0x400;
        }
        D_800B0C04.rot.vx = v;
    }
    if ((func_8008E158(0) & g_s2d_keys[6].mask) || (func_8008E158(0) & 0x2000)) {
        D_800B2D98[0] = 1;
    } else {
        D_800B2D98[0] = 0;
    }
    if ((func_8008E158(0) & g_s2d_keys[5].mask) || (func_8008E158(0) & 0x8000)) {
        D_800B2D98[1] = 1;
    } else {
        D_800B2D98[1] = 0;
    }
    S2dFilterInput(D_800B863C);
    D_800B0C04.rot.vy += (g_filter_sum[0] * 64) >> 12;
    D_800B0C04.rot.vy -= (g_filter_sum[1] * 64) >> 12;
    return 1;
}

/* Whether the party's last six places differ from the first. */
int S2dPartyMoved(void)
{
    int i;

    for (i = 0; i < 6; i++) {
        if (D_800A5088[0][0] != D_800A5088[i][0] ||
            D_800A5088[0][1] != D_800A5088[i][1]) {
            return 1;
        }
    }
    return 0;
}
