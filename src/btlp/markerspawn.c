/* Persona 1 (JP) - building the party's markers.  BTLP only.
 *   0x800A7FB8 BtlSpawnMarkers
 *
 * Six markers stand over the party. The sixth is built first and on its own -
 * four records chained through `attached`, the first two out of the shadow
 * templates - and it is the only one that gets its own position rather than
 * one out of g_btl_marker_pos.
 *
 * The five party markers are two records each, described by g_btl_marker_defs,
 * with the marker's number added to the index so each one takes a different
 * cell of the same model. A last pass puts a frame in front of all five and
 * hangs what was there behind it, which is the same arrangement BtlPickSpawn
 * builds for the six-slot picker: the frame is what g_btl_marker_obj keeps and
 * the piece it carries is reached through `attached`.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/object.h>

#define BTL_MARKERS      5
#define BTL_MARKER_PARTS 2

/* Which of the six object groups the markers live in, and the template the
   sixth marker's last record takes. */
#define MARKER_GROUP 1
#define MARKER_LONE  14

/* Attribute bits: the first goes on every piece of a party marker, the second
   on the frame in front of it, and the third on the frame only. */
#define MARKER_PIECE_BIT 0x400
#define MARKER_FRAME_BIT 0x40000080
#define MARKER_EXTRA_BIT 0x800000

/* Unity, in the two units BtlObjSetScale takes; the frames are drawn at half
   width and height. */
#define MARKER_SCALE_XY 0x80
#define MARKER_SCALE_Z  0x1000

/* Where the sixth marker stands, and how far the third record of it is lifted
   from the two below. */
#define MARKER_LONE_X 0x3D0000
#define MARKER_LONE_Y 0xB80000
#define MARKER_LONE_UP 0x10000

/* What a party marker is made of: a template, the index the marker's number is
   added to, the kind, and the two bytes that land at +0xCD and +0xCE. */
typedef struct {
    /* 0x0 */ const BtlObjDef *defs;
    /* 0x4 */ u_char index;
    /* 0x5 */ u_char kind;
    /* 0x6 */ u_char p7;
    /* 0x7 */ u_char p8;
} BtlMarkerDef;                     /* 8 bytes */

extern const BtlObjDef    g_btl_shadow_defs[];
extern const BtlObjDef    g_btl_obj_defs[];
extern const BtlObjDef    g_btl_lone_defs[];
extern const BtlMarkerDef g_btl_marker_defs[];
extern long               g_btl_marker_pos[][4];
extern BtlObj            *g_btl_marker_obj[];
extern BtlObj            *g_btl_marker_shown[];

#ifdef NON_MATCHING
void BtlSpawnMarkers(void)
{
    const BtlMarkerDef *def;
    const long         *pos;
    BtlObj             *obj;
    BtlObj             *prev;
    long                lone[6];
    int                 marker;
    int                 j;

    lone[0] = MARKER_LONE_X;
    lone[1] = MARKER_LONE_Y;
    lone[2] = 0;
    obj  = BtlObjAlloc(g_btl_shadow_defs, MARKER_GROUP, 0, 1, 0x10, lone, 4, 0x17);
    prev = BtlObjAlloc(g_btl_shadow_defs, MARKER_GROUP, obj, 1, 1, lone, 0xD, 0x1D);
    prev->attached = obj;
    lone[0] -= MARKER_LONE_UP;
    obj = BtlObjAlloc(g_btl_lone_defs, MARKER_GROUP, prev, 3, 10, lone, 0x1F, 0x20);
    obj->attached = prev;
    prev = obj;
    BtlObjSetAttr(prev, MARKER_PIECE_BIT);
    obj = BtlObjAlloc(g_btl_obj_defs, MARKER_GROUP, prev, 1, 0, lone, 0x19, 0x1E);
    obj->attached = prev;
    BtlObjSetAttr(obj, MARKER_FRAME_BIT);
    BtlObjSetScale(obj, MARKER_SCALE_XY, MARKER_SCALE_XY, MARKER_SCALE_Z);
    g_btl_marker_obj[BTL_MARKERS] = obj;

    marker = 0;
    pos = g_btl_marker_pos[0];
    do {
        j = 0;
        def = g_btl_marker_defs;
        prev = 0;
        do {
            j++;
            obj = BtlObjAlloc(def->defs, MARKER_GROUP, prev, def->kind,
                              def->index + marker, pos, def->p7, def->p8);
            def++;
            obj->attached = prev;
            obj->mark_num = marker;
            obj->attr |= MARKER_PIECE_BIT;
            prev = obj;
        } while (j < BTL_MARKER_PARTS);
        g_btl_marker_obj[marker] = obj;
        marker++;
        pos += 4;
    } while (marker < BTL_MARKERS);

    marker = 0;
    prev = 0;
    do {
        obj = BtlObjAlloc(g_btl_obj_defs, MARKER_GROUP, prev, 1, 1,
                          g_btl_marker_pos[marker], 0x19, 0x1E);
        prev = obj;
        obj->mark_num = marker;
        obj->attached = g_btl_marker_obj[marker];
        g_btl_marker_obj[marker] = obj;
        g_btl_marker_shown[marker] = 0;
        marker++;
        obj->attr |= MARKER_EXTRA_BIT;
        BtlObjSetAttr(obj, MARKER_FRAME_BIT);
        BtlObjSetScale(obj, MARKER_SCALE_XY, MARKER_SCALE_XY, MARKER_SCALE_Z);
    } while (marker < BTL_MARKERS);
}
#else
INCLUDE_ASM("btlp/nonmatchings/markerspawn", BtlSpawnMarkers);
#endif

