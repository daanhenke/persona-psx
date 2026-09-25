/* Persona 1 (JP) - a scripted prop scene.  ADV only.
 *   0x800AD680 AdvSpecialProps
 *
 * Script special 2 (AdvScriptSpecial): three records past the room's actors
 * are put in the room as scene-pack sprites, one after another, with a sound
 * cue and a pause of 96 frames between them. The second step drops the first
 * prop in from 96 pixels up; the room shakes (AdvScreenEffect 7) until the
 * third, which stops it.
 */
#include <decomp/types.h>
#include <persona/adv/actor.h>

/* The scene pack's sprites, reached by address. */
#define g_pack_sprites ((void **)0x80100070)

#define P(n)        g_adv_actors[n]
#define PROP_Z      0x3C0
#define PROP_BRIGHT 0x80
#define WAIT        0x60

extern void SlotClear(u_char slot);
extern void SlotInit(void *def, u_char slot, int attr, short x, short y);
extern void SlotInitTagged(void *def, u_char slot, int attr, short x,
                           short y);
extern void SlotSetBrightness(u_char slot, u_char level);
extern void ActorSetTile(short x, short y, AdvActor *a);
extern void AdvScreenEffect(u_char n);
extern void AdvSoundCommand(short cmd);
extern void AdvRunFrame(void);
extern void ViewShakeStop(void);

/* Record `n` as a still prop of pack sprite `kind` on tile (x, y). */
static inline void PropPlace(u_char n, u_char kind, u_char x, u_char y)
{
    P(n).kind = kind;
    P(n).x = x;
    P(n).y = y;
    P(n).flags = 0;
    P(n).bright = PROP_BRIGHT;
    P(n).lift = 0;
    P(n).next_dir = 0;
    P(n).dir = 0;
    P(n).phase = 0;
    P(n).slope = 0;
    P(n).move = MOVE_NONE;
    ActorSetTile(x, y, &P(n));
}

void AdvSpecialProps(void)
{
    int i;

    SlotClear(0x12);
    AdvScreenEffect(7);
    AdvSoundCommand(0x30);

    PropPlace(16, 0x8C, 0x13, 2);
    P(16).z = PROP_Z;
    /* Signed arithmetic: as u_short the decrements fold into additions of
       0xFFFC and 0xFFFF, which the original does not have. */
    SlotInit(g_pack_sprites[12], 16, PROP_Z,
             P(16).world_x = (short)P(16).world_x - 4,
             P(16).world_y = (short)P(16).world_y - 1);
    SlotSetBrightness(16, P(16).bright);
    P(16).id = 0;

    PropPlace(17, 0x8D, 0xF, 0xB);
    P(17).z = PROP_Z;
    SlotInit(g_pack_sprites[13], 17, PROP_Z, P(17).world_x += 1,
             P(17).world_y);
    SlotSetBrightness(17, P(17).bright);
    P(17).id = 0;

    for (i = 0; i < WAIT; i++) {
        AdvRunFrame();
    }

    AdvSoundCommand(0x2F);
    PropPlace(16, 0x8A, 0x10, 3);
    SlotInitTagged(g_pack_sprites[10], 16, P(16).z, P(16).world_x,
                   P(16).world_y);
    SlotSetBrightness(16, P(16).bright);
    P(16).id = 0;
    P(16).world_y += WAIT;
    AdvRunFrame();
    for (i = 0; i < WAIT; i++) {
        P(16).world_y = (short)P(16).world_y - 1;
        AdvRunFrame();
    }

    ViewShakeStop();
    AdvSoundCommand(0x31);
    PropPlace(17, 0x8E, 0x10, 3);
    SlotInit(g_pack_sprites[14], 17, P(17).z, P(17).world_x, P(17).world_y);
    SlotSetBrightness(17, P(17).bright);
    P(17).id = 0;
    PropPlace(18, 0x8E, 0x10, 6);
    SlotInit(g_pack_sprites[14], 18, P(18).z, P(18).world_x, P(18).world_y);
    SlotSetBrightness(18, P(18).bright);
    P(18).id = 0;
    P(17).world_x += 2;
    P(18).world_x += 6;
    P(17).world_y -= 10;
    P(18).world_y -= 6;
    for (i = 0; i < WAIT; i++) {
        AdvRunFrame();
    }
}
