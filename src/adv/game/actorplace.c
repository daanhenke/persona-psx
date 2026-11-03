/* Persona 1 (JP) - putting a scripted actor in the room.  ADV only.
 *   0x800ADCFC ActorPlace    0x800AE040 AdvScreenEffect
 *
 * Script command 64 fills in an actor record and this puts it on screen: on
 * its tile, with the sprite its kind calls for. A kind of 0xFF takes the
 * actor away instead. Kinds from 0x80 up are sprites out of the scene pack,
 * kind 1 is a party member standing, and the rest are the overlay's own
 * sprites, one for each facing.
 *
 * Command 6C, the screen effects, sits alongside: fades of the whole screen
 * and of the box at three speeds, two flashes, and the view shakes, which
 * are anything else - a scene asks for 7, 8 or 9 (viewshake.c).
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/adv/actor.h>
#include <persona/common/slot.h>

#define g_slots ((Slot *)0x800DC10C)

/* The scene pack's sprites, reached by address. */
#define g_pack_sprites ((void **)0x80100070)

#define KIND_NONE   0xFF
#define KIND_PACK   0x80
#define KIND_STAND  1
#define ACTOR_UNTAG 0x80   /* a pack sprite that does not take the tag */
#define SHADOW_SLOT 24

extern Slot   *g_slot_cur;
extern void   *g_actor_defs[];   /* by kind and facing */
extern u_char  g_dir_flip[];
extern int     g_actor_dim;

extern void ActorSetTile(short x, short y, AdvActor *a);
extern void SlotInit(void *def, u_char slot, int attr, short x, short y);
extern void ActorSetStandSprite(u_char a);
extern void ActorSetShadowSprite(u_char a);
extern void SlotSetBrightness(u_char slot, u_char level);
extern void AdvFadeUpBlocking(short step, short limit);
extern void AdvFadeDownBlocking(short step, short floor);
extern void AdvBoxFadeUp(short step, short from, short to, short x, u_short y,
                         short char_frame);
extern void AdvBoxFadeDown(short step, short from, short to, short x,
                           u_short y, short char_frame);

extern u_short g_view_shake;

#define A g_adv_actors[a]

/* Compute the shadow slot up front, as in the original. The final word-sized
   index keeps the actor lookup live across the palette and tag stores. */
void ActorPlace(u_char a)
{
    u_char kind;
    unsigned int index;
    Slot  *t = &g_slots[a + SHADOW_SLOT];

    if (A.kind != KIND_NONE) {
        ActorSetTile(A.x, A.y, &A);
        A.world_x++;
        A.world_y -= 2;
        A.id = 0;
        if (A.kind >= KIND_PACK) {
            kind = A.kind - KIND_PACK;
            if (A.flags & ACTOR_UNTAG) {
                SlotInit(g_pack_sprites[kind], a, A.z, A.world_x, A.world_y);
            } else {
                SlotInitTagged(g_pack_sprites[kind], a, A.z, A.world_x,
                               A.world_y);
            }
            ActorSetShadowSprite(a);
        } else if (A.kind == KIND_STAND) {
            ActorSetStandSprite(a);
        } else {
            kind = A.kind * 4 + A.dir;
            SlotInit(g_actor_defs[kind], a, A.z, A.world_x, A.world_y);
            ActorSetShadowSprite(a);
            if (g_dir_flip[A.dir]) {
                g_slot_cur = &g_slots[a];
                g_slot_cur->attr |= SLOT_ATTR_XSCALE;
                g_slot_cur += SHADOW_SLOT;
                g_slot_cur->attr |= SLOT_ATTR_XSCALE;
            }
        }
        index = a;
        g_slot_cur = &g_slots[index];
        g_slot_cur->clut_y = a;
        g_slot_cur->tpage_add = a;
        SlotSetBrightness(index, g_adv_actors[index].bright
            - ((g_adv_actors[index].bright >> 4) << 3) * g_actor_dim);
    } else {
        A.id = ACTOR_NONE;
    }
}

void AdvScreenEffect(u_char n)
{
    switch (n) {
    case 1:
        AdvFadeUpBlocking(4, 0x80);
        break;
    case 2:
        AdvFadeUpBlocking(2, 0x80);
        break;
    case 3:
        AdvFadeUpBlocking(1, 0x80);
        break;
    case 4:
        AdvFadeDownBlocking(4, 0);
        break;
    case 5:
        AdvFadeDownBlocking(2, 0);
        break;
    case 6:
        AdvFadeDownBlocking(1, 0);
        break;
    case 10:
        AdvBoxFadeDown(4, 0xFF, 0x80, 0, 0, 0);
        break;
    case 11:
        AdvBoxFadeDown(2, 0xFF, 0x80, 0, 0, 0);
        break;
    case 12:
        AdvBoxFadeDown(1, 0xFF, 0x80, 0, 0, 0);
        break;
    case 13:
        AdvBoxFadeUp(4, 0x80, 0xFF, 0, 0, 0);
        break;
    case 14:
        AdvBoxFadeUp(2, 0x80, 0xFF, 0, 0, 0);
        break;
    case 15:
        AdvBoxFadeUp(1, 0x80, 0xFF, 0, 0, 0);
        break;
    case 16:
        AdvFadeUpBlocking(4, 0xFF);
        AdvFadeDownBlocking(4, 0x80);
        break;
    case 17:
        AdvFadeUpBlocking(8, 0xFF);
        AdvFadeDownBlocking(8, 0x80);
        break;
    default:
        g_view_shake = n;
        break;
    }
}
