/* Persona 1 (JP) - putting a member's marker back up.  BTLP only.
 *   0x800AC440 BtlShowMarker     0x800AC5E8 BtlHideMarkers
 *   0x800AC6E0 BtlRestoreMarkers
 *
 * A marker is the pair of records BtlSpawnMarkers built for a slot: the one
 * g_btl_marker_obj keeps and whatever hangs behind it. BtlShowMarker arms the
 * front one with the standing script, works the row of positions out again so
 * the marker lands over whoever is there now, and then builds a second record
 * in front of the whole thing carrying the kind the caller asked for. Both
 * are given the same motion and phase, so they arrive together.
 *
 * The second argument is the odd one: zero means the member can act, and it
 * picks both the sound the marker arrives with and which of two pictures the
 * new record takes. A member who cannot act also gets a script of their own
 * out of g_btl_marker_scripts, chosen by the kind, and a bit set on the
 * record that says so.
 *
 * The two sweeps below walk the party and put a marker back up for anyone who
 * wants one, and they agree on who is eligible: the slot is filled, the member
 * is not down, and they are not flagged out of the fight. What they disagree
 * on is the fourth test and what kind of marker goes up - BtlHideMarkers takes
 * any marker that is up at all and replaces it with the member's own ailment
 * or their mark kind, depending on whether a negotiation has just ended;
 * BtlRestoreMarkers takes only the ones parked at 4, clears the flag that
 * parked them, and puts up the plain kind.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/sound.h>

#define BTL_MARKERS 5

/* The marker state BtlRestoreMarkers is looking for, and the flag that goes
   with it. */
#define MARKER_PARKED 4
#define MARKER_FLAG   0x2000

/* Which sound bank the marker's arrival comes out of, and the two pictures
   the new record is drawn from. */
#define MARKER_BANK    2
#define MARKER_PICTURE 4

/* Attribute bits: the piece bit on the record behind, the one every marker
   carries, and the one that says the member cannot act. */
#define MARKER_PIECE_BIT  0x400
#define MARKER_OWN_BIT    0x40
#define MARKER_NOACT_BIT  0x100

/* How far in front of the marker the new record stands. */
#define MARKER_LIFT 0x210000

/* The pair every record of a marker is given. */
#define MARKER_UNK_CD 0x19
#define MARKER_UNK_CE 0x1E

/* Which of the six object groups a marker lives in, and the motion it arrives
   with. */
#define MARKER_GROUP  1
#define MARKER_MOTION 2

extern BtlObjDef   g_btl_obj_defs[];
extern BtlObj     *g_btl_marker_obj[];
extern BtlObj     *g_btl_marker_shown[];
extern BtlSeqStep  g_btl_marker_stand[];
extern BtlSeqStep *g_btl_marker_scripts[];
extern u_char      g_btl_talk_outcome;

extern void BtlPlaceMemberMarkers(int slot, int row);

void BtlShowMarker(int slot, int blocked, int kind)
{
    BtlObj *obj;
    BtlObj *back;
    long    pos[3];
    int     seq;

    seq = blocked == 0;
    BtlSePlay(MARKER_BANK, seq);
    BtlObjSetScript(g_btl_marker_obj[slot], g_btl_marker_stand);
    BtlPlaceMemberMarkers(slot, 1);
    back = BtlObjLast(g_btl_marker_obj[slot]);
    back->attr |= MARKER_PIECE_BIT;
    back->scale_to = kind;
    back->x -= MARKER_LIFT;
    g_btl_obj_prev->attached = 0;

    pos[0] = g_btl_marker_obj[slot]->x - MARKER_LIFT;
    pos[1] = g_btl_marker_obj[slot]->y;
    pos[2] = 0;
    obj = BtlObjAlloc(g_btl_obj_defs, MARKER_GROUP, 0, 1, seq + MARKER_PICTURE,
                      pos, MARKER_UNK_CD, MARKER_UNK_CE);
    obj->attr |= MARKER_OWN_BIT;
    BtlObjMoveBefore(g_btl_marker_obj[slot], obj);
    obj->attached = back;
    obj->mark_num = slot;
    obj->motion   = MARKER_MOTION;
    back->motion  = MARKER_MOTION;
    obj->phase    = 0;
    back->phase   = 0;
    if (seq != 0) {
        BtlObjSetScript(back, g_btl_marker_scripts[kind]);
        obj->attr |= MARKER_NOACT_BIT;
    } else {
        obj->attr &= ~MARKER_NOACT_BIT;
    }
    g_btl_marker_shown[slot] = obj;
}

void BtlHideMarkers(void)
{
    int i;
    int kind;

    i = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && g_btl_actors[i].marker != 0) {
            g_btl_actors[i].marker = 0;
            if (g_btl_talk_outcome != 0) {
                kind = g_btl_actors[i].mark_kind;
            } else {
                kind = g_btl_actors[i].c.unk5D & 0xF;
            }
            BtlShowMarker(i, 0, kind);
        }
        i++;
    } while (i < BTL_MARKERS);
}

void BtlRestoreMarkers(void)
{
    int i;

    i = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && g_btl_actors[i].marker == MARKER_PARKED) {
            g_btl_actors[i].flags &= ~MARKER_FLAG;
            g_btl_actors[i].marker = 0;
            BtlShowMarker(i, 0, -(g_btl_actors[i].unkC8 == 0) & 3);
        }
        i++;
    } while (i < BTL_MARKERS);
}
