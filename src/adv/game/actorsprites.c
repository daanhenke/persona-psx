/* Persona 1 (JP) - putting the field's actors on screen.  ADV only.
 *   ADV 0x800826F8
 *
 * Once a frame every actor record is turned into its sprite slot: placed where
 * its world position falls once the camera and the view offset are taken off,
 * raised seven pixels for every step it stands above the floor, and sorted at
 * its own depth. The eight actors a room defines have a second sprite under
 * them - a shadow or a reflection, as their `shadow` byte says - and may be
 * drawn mirrored or semi-transparent.
 *
 * Record 24 is the party's own leader and has a slot of its own away from the
 * others, as does the marker drawn over the first actor; the eight records
 * after the leader's are placed without either extra.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/adv/actor.h>
#include <persona/common/slot.h>

#define g_slots ((Slot *)0x800DC10C)

#define LEADER        24
#define LEADER_SLOT   0x32
#define SHADOW_SLOT   0x18   /* the eight room actors' second sprites  */
#define OUTLINE_SLOT  0x20   /* and a third, sorted just in front      */
#define EXTRA_SLOT    0x40   /* the eight records after the leader's  */
#define EXTRA_ACTOR   25
#define MARK_SLOT     0x34
#define ROOM_ACTORS   8

#define STEP_H        7
#define SHADOW_DROP   0x34
#define SHADOW_SCALE  0x2AA  /* a third of full height */
#define SLOT_ATTR_2000 0x2000

extern Slot  *g_slot_cur;
extern u_short g_cam_x;
extern u_short g_cam_y;
extern u_short g_view_dx;
extern u_short g_view_dy;

/* Whether each facing is drawn mirrored. */
extern u_char g_dir_flip[];

#define SCREEN_X(n) (g_adv_actors[n].world_x - g_cam_x - g_view_dx)
#define SCREEN_Y(n)                                                            \
    (g_adv_actors[n].world_y - g_cam_y - g_view_dy                             \
     - g_adv_actors[n].unk26 * STEP_H)
#define DEPTH(n) (g_adv_actors[n].z + g_adv_actors[n].depth)

/* 87.90%: the original re-masks the counter at every use rather than keeping
   a masked copy across the calls, which is what lets its shadow cases share
   their tails; the y terms also load in a different order. */
#ifdef NON_MATCHING
void ActorsPlaceSprites(void)
{
    u_char i;

    for (i = 0; i < ACTOR_COUNT; i++) {
        if (g_adv_actors[i].id != ACTOR_NONE) {
            if (i == LEADER) {
                SlotSetPos(LEADER_SLOT, g_adv_actors[LEADER].z,
                           SCREEN_X(LEADER), SCREEN_Y(LEADER));
            } else {
                SlotSetPos(i, DEPTH(i), SCREEN_X(i), SCREEN_Y(i));
            }

            if (i < ROOM_ACTORS) {
                SlotSetPos(i + OUTLINE_SLOT, DEPTH(i) - 1, SCREEN_X(i),
                           SCREEN_Y(i));
                g_slot_cur = &g_slots[i + SHADOW_SLOT];
                g_slot_cur->tpage_add = i;
                switch (g_adv_actors[i].shadow) {
                case SHADOW_NONE:
                    SlotClear(i + SHADOW_SLOT);
                    break;
                case SHADOW_FLAT:
                    SlotSetPos(i + SHADOW_SLOT, DEPTH(i), SCREEN_X(i),
                               SCREEN_Y(i));
                    g_slot_cur->scale_y = SHADOW_SCALE;
                    g_slot_cur->attr |= SLOT_ATTR_2000;
                    SlotSetSemiTrans(i + SHADOW_SLOT, 1);
                    break;
                case SHADOW_FLAT_LOW:
                    SlotSetBrightness(i + SHADOW_SLOT,
                                      g_adv_actors[i].bright * 3 / 2);
                    SlotSetPos(i + SHADOW_SLOT, DEPTH(i), SCREEN_X(i),
                               SCREEN_Y(i) + SHADOW_DROP);
                    g_slot_cur->scale_y = SHADOW_SCALE;
                    g_slot_cur->attr |= SLOT_ATTR_2000;
                    SlotSetSemiTrans(i + SHADOW_SLOT, 1);
                    break;
                case SHADOW_COPY:
                    SlotSetPos(i + SHADOW_SLOT, DEPTH(i), SCREEN_X(i),
                               SCREEN_Y(i));
                    SlotSetSemiTrans(i + SHADOW_SLOT, 1);
                    break;
                case SHADOW_COPY_LIT:
                    SlotSetBrightness(i + SHADOW_SLOT,
                                      g_adv_actors[i].bright * 3 / 2);
                    SlotSetPos(i + SHADOW_SLOT, DEPTH(i), SCREEN_X(i),
                               SCREEN_Y(i));
                    SlotSetSemiTrans(i + SHADOW_SLOT, 1);
                    break;
                }

                g_slot_cur = &g_slots[i];
                if ((g_adv_actors[i].flags & ACTOR_FLIP_OK)
                    && g_dir_flip[g_adv_actors[i].dir]) {
                    g_slot_cur->attr |= SLOT_ATTR_XSCALE;
                    g_slot_cur += SHADOW_SLOT;
                    g_slot_cur->attr |= SLOT_ATTR_XSCALE;
                } else {
                    g_slot_cur->attr &= ~SLOT_ATTR_XSCALE;
                    g_slot_cur += SHADOW_SLOT;
                    g_slot_cur->attr &= ~SLOT_ATTR_XSCALE;
                }
            }

            if (g_adv_actors[i].flags & ACTOR_SEMITRANS) {
                SlotSetSemiTrans(i, 1);
            } else {
                SlotSetSemiTrans(i, 0);
            }
        } else {
            SlotClear(i == LEADER ? LEADER_SLOT : i);
            if (i < ROOM_ACTORS) {
                SlotClear(i + SHADOW_SLOT);
            }
        }
    }

    for (i = 0; i < ROOM_ACTORS; i++) {
        SlotSetPos(i + EXTRA_SLOT, g_adv_actors[EXTRA_ACTOR + i].z,
                   SCREEN_X(EXTRA_ACTOR + i), SCREEN_Y(EXTRA_ACTOR + i));
    }
    SlotSetPos(MARK_SLOT, g_adv_actors[0].z, SCREEN_X(0), SCREEN_Y(0) + 4);
}
#else
INCLUDE_ASM("adv/nonmatchings/game/actorsprites", ActorsPlaceSprites);
#endif
