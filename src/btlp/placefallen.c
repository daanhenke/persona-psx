/* Persona 1 (JP) - putting the fallen back on the field.  BTLP only.
 *   0x800A735C BtlPlaceFallen  0x800A7560 BtlPlaceFallenStep
 *
 * The last thing BtlRestoreField does. The pick grid is put up over the field,
 * and every party member who is down or out of the fight is taken in turn: a
 * line naming them goes up, their marker is lit, and the player walks a cursor
 * round the formation until they are stood somewhere they are allowed to be.
 * Then the grid comes down again once everyone has settled.
 *
 * BtlPlaceFallenStep is one frame of that cursor. The directions walk the
 * row and the column round the grid, wrapping at either edge, and the grid's
 * anchor is kept over the cell. A confirm on an empty cell whose four
 * neighbours are empty too puts the member there: back in the formation, in
 * full colour, with no ailment and at least one hit point. It answers 1 once
 * the member is placed and BTL_PICK_WAIT until then.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/status.h>
#include <persona/btlp/text.h>

/* What the chosen member's marker is lit with, the name slot of the line,
   and where the line goes. */
#define MARK_CHOSEN  0x1000000
#define PLACE_INSERT 4
#define PLACE_LINE_X 0x10
#define PLACE_LINE_Y 0x10

/* The colour a member comes back at, and the fade that puts it there. */
#define PLACE_LIT  0x80
#define PLACE_FADE 0xFF

/* How long the grid is given to come in, and to go again. */
#define PLACE_GRID_IN  30
#define PLACE_GRID_OUT 60

extern BtlObj  *g_btl_pick_cursors[];
extern u_char   g_btl_place_line[];

/* 99.26%, registers only: the slot counter, the marker and the record offset
   take s2, s1 and s0 in the image and a rotation of those here. Declaring them
   in another order and walking the markers by pointer change nothing. */
#ifdef NON_MATCHING
void BtlPlaceFallen(void)
{
    int       i;
    BtlActor *a;

    BtlShowAilmentMarks(0);
    BtlSpawnPickGrid();
    BtlSePlay(PLACE_SE_SLOT, PLACE_SE_OPEN);
    g_btl_delay = PLACE_GRID_IN;
    for (;;) {
        BtlDrawFrame();
        if (g_btl_delay == 0) {
            break;
        }
    }
    while (g_btl_pick_cursors[0]->motion != 0) {
        BtlDrawFrame();
    }

    i = 0;
    do {
        a = &g_btl_actors[i];
        if (g_btl_actors[i].c.key == 0
            || (signed char)g_btl_actors[i].c.status == BTL_STATUS_DOWN
            || (g_btl_actors[i].flags & BTL_ACTOR_OUT) != 0) {
            if (a->c.key != 0) {
                BtlSetInsert(PLACE_INSERT, a->c.name);
                BtlOpenMessage(0, 0, g_btl_place_line, PLACE_LINE_X,
                               PLACE_LINE_Y);
                g_btl_marker_obj[i]->attr |= MARK_CHOSEN;
                g_btl_place_member = i;
                while (BtlPlaceFallenStep() < 0) {
                    BtlDrawFrame();
                }
                g_btl_marker_obj[i]->attr &= ~MARK_CHOSEN;
                BtlPartyResetGfx();
            }
        }
        BtlDrawFrame();
        i++;
    } while (i < BTL_PARTY);

    BtlAfterTalk();
    for (;;) {
        if (BtlActorsIdle() != 0) {
            break;
        }
        BtlDrawFrame();
    }
    BtlCloseMessage(0);
    BtlDespawnPickGrid();
    i = 0;
    BtlSePlay(PLACE_SE_SLOT, PLACE_SE_SHUT);
    do {
        BtlDrawFrame();
        i++;
    } while (i < PLACE_GRID_OUT);
}
#else
INCLUDE_ASM("btlp/nonmatchings/placefallen", BtlPlaceFallen);
#endif

/* 85.95%: the image steps down and right without a branch - the next row is
   masked with the negated test, the flag gcc makes for "keep it or clear it" -
   where gcc here branches round a clear however the step is written: as a
   ternary, as an if on a short local, or as the mask itself. It also loads the
   record's flags before it clears the ailment, and gcc here after. */
#ifdef NON_MATCHING
int BtlPlaceFallenStep(void)
{
    int next;
    int   keys;
    int   cell;

    g_btl_grid_anchor->attr &= ~BTL_OBJ_HIDDEN;
    keys = BtlMenuKey();
    if (keys & PAD_DIRS) {
        BtlSePlay(1, 0);
    }
    if (keys & PAD_UP) {
        next = (unsigned short)g_btl_place_row - 1;
        g_btl_place_row = next;
        if ((short)next < 0) {
            next = GRID_H - 1;
            g_btl_place_row = next;
        }
    }
    if (keys & PAD_DOWN) {
        next = (unsigned short)g_btl_place_row;
        next = next + 1;
        next &= -((short)next < GRID_H);
        g_btl_place_row = next;
    }
    if (keys & PAD_LEFT) {
        next = (unsigned short)g_btl_place_col - 1;
        g_btl_place_col = next;
        if ((short)next < 0) {
            next = GRID_W - 1;
            g_btl_place_col = next;
        }
    }
    if (keys & PAD_RIGHT) {
        next = (unsigned short)g_btl_place_col;
        next = next + 1;
        next &= -((short)next < GRID_W);
        g_btl_place_col = next;
    }

    g_btl_grid_anchor->x = (g_btl_place_col * PLACE_XPITCH + PLACE_X) << 16;
    g_btl_grid_anchor->y = (g_btl_place_row * PLACE_YPITCH + PLACE_Y) << 16;
    cell = g_btl_place_row * GRID_W + g_btl_place_col;
    g_btl_place_cell = g_btl_formation[cell];
    if ((g_btl_pad1_edge & g_btl_key_confirm) == 0) {
        return BTL_PICK_WAIT;
    }
    if (g_btl_place_cell != CELL_EMPTY) {
        return BTL_PICK_WAIT;
    }
    if (BtlFormationCellFree(g_btl_place_col, g_btl_place_row) != 0) {
        BtlSePlay(PLACE_SE_SLOT, PLACE_SE_PUT);
        g_btl_formation[cell] = g_btl_place_member;
        BtlPlaceMember(g_btl_place_member, g_btl_place_col, g_btl_place_row);
        g_btl_actors[g_btl_place_member].obj->attr &= ~BTL_OBJ_HIDDEN;
        g_btl_actors[g_btl_place_member].obj->rgb_to[0] = PLACE_LIT;
        g_btl_actors[g_btl_place_member].obj->rgb_to[1] = PLACE_LIT;
        g_btl_actors[g_btl_place_member].obj->rgb_to[2] = PLACE_LIT;
        g_btl_actors[g_btl_place_member].obj->fade = PLACE_FADE;
        g_btl_actors[g_btl_place_member].flags &= ~BTL_ACTOR_OUT;
        g_btl_actors[g_btl_place_member].c.status = 0;
        g_btl_actors[g_btl_place_member].c.ail_level = 0;
        if (g_btl_actors[g_btl_place_member].c.hp == 0) {
            g_btl_actors[g_btl_place_member].c.hp = 1;
        }
        BtlPartyResetGfx();
        BtlBuildMarkers();
        BtlRefreshPickCursors();
        return 1;
    }
    return BTL_PICK_WAIT;
}
#else
INCLUDE_ASM("btlp/nonmatchings/placefallen", BtlPlaceFallenStep);
#endif
