/* Persona 1 (JP) - the script opcode that shrinks a slot away.
 *
 *   ADV @ 0x800AF568
 *
 * Event script opcode 0x68. Slot 45 goes back to full scale about a centre of
 * (0x2C, 0x30) and then loses 0x200 off both scales a frame until the
 * horizontal one is down to 1 - sixteen frames from full size to gone - and
 * the slot is cleared. A frame is drawn between steps, so the call blocks for
 * the whole animation.
 */
#include <types.h>
#include <persona/common/slot.h>

#define g_slots ((Slot *)0x800DC10C)

#define SHRINK_SLOT 45
#define FULL_SCALE  0x1000
#define SHRINK_STEP 0x200

extern void AdvRunFrame(void);
extern void SlotClear(u_char slot);

void AdvShrinkSlot(void)
{
    Slot *s;

    s = &g_slots[SHRINK_SLOT];
    s->scale_x = FULL_SCALE;
    s->scale_y = FULL_SCALE;
    s->mx = 0x2C;
    s->my = 0x30;
    do {
        AdvRunFrame();
        s->scale_x -= SHRINK_STEP;
        if (s->scale_x < 1) {
            s->scale_x = 1;
        }
        s->scale_y -= SHRINK_STEP;
        if (s->scale_y < 1) {
            s->scale_y = 1;
        }
    } while (s->scale_x > 1);
    AdvRunFrame();
    SlotClear(SHRINK_SLOT);
}
