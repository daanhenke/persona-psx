/* Persona 1 (JP) - running a field scene for one frame.  ADV @ 0x8007E6DC.
 *
 * The overlay's entry loop calls this every frame while it is in field mode.
 * The scene has to be built before it can be walked in, so the first state
 * does that and steps itself on; from then on every frame is a walking frame.
 *
 * The entry loop builds the scene itself on the way in, which is why the state
 * decides here rather than the builder being called unconditionally.
 */
#include <types.h>

/* Before the scene is built, and after. */
#define FIELD_BUILD 0
#define FIELD_WALK  1

extern short g_adv_field_state;

extern void AdvFieldEnter(void);
extern void AdvFieldRun(void);

void AdvFieldTick(void)
{
    switch (g_adv_field_state) {
    case FIELD_BUILD:
        AdvFieldEnter();
        g_adv_field_state++;
        break;
    case FIELD_WALK:
        AdvFieldRun();
        break;
    }
}
