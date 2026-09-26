/* Persona 1 (JP) - putting a room's actors on screen.  ADV @ 0x80084694.
 *
 * Called whenever the room is (re)built - on entry, after a menu and at the
 * end of the TYN cutscene - once AdvBuildActors has filled the records. Every
 * record is placed on its tile (ActorSetTile's projection, copied in here)
 * and given its sprite:
 *
 *  - the room's own actors (0-7): the pack's sprite for kinds from 0x80 up,
 *    the standing party sprite for kind 1, the overlay's own set otherwise;
 *    each takes its record's number as its CLUT row and page offset, and its
 *    shadow;
 *  - the props (16-23), from the pack;
 *  - the marks (8-15), one standing sprite per scene, moved on a quarter
 *    turn when the mark's flag is already set;
 *  - the spots (25-32), from a table of sixteen-byte sprites.
 *
 * A record with no kind (0xFF) is marked unused. The leader's own record, 24,
 * is emptied at the end.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/adv/actor.h>
#include <persona/adv/room.h>
#include <persona/adv/scene.h>
#include <persona/common/slot.h>

#define MARK_ACTOR 8
#define PROP_ACTOR 16
#define LEADER     24
#define SPOT_ACTOR 25
#define GROUP      8

#define KIND_NONE   0xFF
#define KIND_PACK   0x80
#define KIND_PARTY  1
#define KIND_PLAIN  11    /* the overlay's own kinds below this are drawn
                             plain, the rest tagged */
#define ACTOR_PLAIN 0x80  /* flags: draw a tagged kind plain */

#define PROP_SLOT 0x10
#define SPOT_SLOT 0x40
#define SPOT_Z    0x3BE

/* Reached by hardcoded address rather than through the linker symbol. */
#define g_slots        ((Slot *)0x800DC10C)
#define g_pack_sprites ((void **)0x80100070)
#define g_flag_bank1   ((u_char *)0x801F2A08)

#define BANK1_SET(id) (g_flag_bank1[(id) / 8] & (1 << ((id) & 7)))

typedef struct {
    /* 0x0 */ short   unk0;
    /* 0x2 */ u_char  kind;
    /* 0x3 */ u_char  pad03;
    /* 0x4 */ short   flag;
    /* 0x6 */ u_short payload;
    /* 0x8 */ u_char  pad08[2];
} AdvChest;

#define g_chest_defs ((AdvChest *)0x80100BD0)

extern Slot   *g_slot_cur;
extern void   *g_actor_defs[];   /* by kind * 4 + facing */
extern void   *g_stand_defs[];
extern u_char  g_spot_sprites[][16];

extern void SlotInit(void *def, u_char slot, int attr, short x, short y);
extern void SlotInitTagged(void *def, u_char slot, int attr, short x,
                           short y);
extern void SlotSetBrightness(u_char slot, u_char bright);
extern void ActorSetStandSprite(u_char a);
extern void ActorSetShadowSprite(short a);

/* The actor record as this source declares it: the screen position is
   signed here. */
typedef struct {
    /* 0x00 */ int     script;
    /* 0x04 */ u_int   flags;
    /* 0x08 */ u_char *move;
    /* 0x0C */ u_short id;
    /* 0x0E */ short   world_x;
    /* 0x10 */ short   world_y;
    /* 0x12 */ short   z;
    /* 0x14 */ short   depth;
    /* 0x16 */ u_char  face;
    /* 0x17 */ u_char  dir;
    /* 0x18 */ u_char  next_dir;
    /* 0x19 */ u_char  phase;
    /* 0x1A */ signed char steps;
    /* 0x1B */ u_char  bright;
    /* 0x1C */ u_char  x, y;
    /* 0x1E */ u_char  next_x, next_y;
    /* 0x20 */ u_char  home_x, home_y;
    /* 0x22 */ u_char  kind;
    /* 0x23 */ u_char  unk23;
    /* 0x24 */ u_char  pad24[8];
} PlacedActor;

#define g_placed ((PlacedActor *)0x801F15D8)

static inline void PlaceTile(short x, short y, PlacedActor *a)
{
    short   d;
    u_short ox;
    u_short oy;

    ox = g_room_origin_x + x * TILE_X;
    d = DEPTH_BASE - y;
    d = d - (ROOM_W - x) * 32;
    oy = g_room_origin_y;
    a->world_x = ox + y * TILE_X + y / 2;
    a->world_y = oy + y * TILE_Y - x * TILE_Y;
    a->phase = 0;
    a->z = d;
    a->x = x;
    a->y = y;
}

#define A g_placed[i]
#define P g_placed[PROP_ACTOR + i]
#define M g_placed[MARK_ACTOR + i]
#define S g_placed[SPOT_ACTOR + i]

#ifdef NON_MATCHING
void AdvPlaceActors(void)
{
    u_char i;
    AdvChest *c;

    i = 0;
actors:

        if (A.kind != KIND_NONE) {
            PlaceTile(A.x, A.y, &A);
            A.id = 0;
            A.world_x += 1;
            A.world_y -= 2;
            if (A.kind >= KIND_PACK) {
                if (A.flags & ACTOR_PLAIN) {
                    SlotInit(g_pack_sprites[(u_char)(A.kind - KIND_PACK)], i,
                             A.z, A.world_x, A.world_y);
                } else {
                    SlotInitTagged(g_pack_sprites[(u_char)(A.kind - KIND_PACK)],
                                   i, A.z, A.world_x, A.world_y);
                }
            } else if (A.kind == KIND_PARTY) {
                ActorSetStandSprite(i);
                i++;
                goto next;
            } else if ((A.flags & ACTOR_PLAIN) || A.kind < KIND_PLAIN) {
                SlotInit(g_actor_defs[A.kind * 4 + A.dir], i, A.z,
                         A.world_x + 4, A.world_y - 2);
            } else {
                SlotInitTagged(g_actor_defs[A.kind * 4 + A.dir], i, A.z,
                               A.world_x + 4, A.world_y - 2);
            }
            g_slot_cur = &g_slots[i];
            g_slot_cur->clut_y = i;
            g_slot_cur->tpage_add = i;
            SlotSetBrightness(i, A.bright);
            ActorSetShadowSprite(i++);
        } else {
            A.id = ACTOR_NONE;
            i++;
        }
    next:
    if (i < GROUP) goto actors;


    i = 0;
props:

        if (P.kind != KIND_NONE) {
            PlaceTile(P.x, P.y, &P);
            if (P.unk23) {
                SlotInit(g_pack_sprites[(u_char)(P.kind - KIND_PACK)],
                         PROP_SLOT + i, P.z, P.world_x, P.world_y);
            } else {
                SlotInitTagged(g_pack_sprites[(u_char)(P.kind - KIND_PACK)],
                               PROP_SLOT + i, P.z, P.world_x, P.world_y);
            }
            SlotSetBrightness(PROP_SLOT + i, P.bright);
            P.id = 0;
            i++;
        } else {
            P.id = ACTOR_NONE;
            i++;
        }
    if (i < GROUP) goto props;


    i = 0;
marks:

        if (M.x != 0xFF && M.y != 0xFF) {
            c = &g_chest_defs[i];
            M.move = MOVE_NONE;
            PlaceTile(M.x, M.y, &M);
            M.world_x += 3;
            M.world_y -= 4;
            SlotInitTagged(g_stand_defs[64 + g_adv_scene->pad25],
                           MARK_ACTOR + i, M.z, M.world_x, M.world_y);
            M.id = 0;
            if (BANK1_SET(c->flag)) {
                g_slot_cur = &g_slots[MARK_ACTOR + i];
                g_slot_cur->u_add = 0x30;
            }
            i++;
        } else {
            M.id = ACTOR_NONE;
            i++;
        }
    if (i < GROUP) goto marks;


    i = 0;
spots:

        if (S.dir != KIND_NONE) {
            PlaceTile(S.x, S.y, &S);
            SlotInitTagged(g_spot_sprites[S.dir], SPOT_SLOT + i, SPOT_Z,
                           S.world_x, S.world_y);
        }
        i++;
    if (i < GROUP) goto spots;

    g_placed[LEADER].id = ACTOR_NONE;
}
#else
INCLUDE_ASM("adv/nonmatchings/game/placeactors", AdvPlaceActors);
#endif
