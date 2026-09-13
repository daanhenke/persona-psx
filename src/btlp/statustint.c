/* Persona 1 (JP) - colouring an actor for its ailment.  BTLP only.
 *   0x800C5D84 BtlObjStatusTint
 *
 * An actor's object keeps a pointer back to the actor it belongs to, so the
 * ailment is read straight out of the Char rather than passed in. Two ailments
 * tint - 4 blue and 5 green - and both put the shadow back if something had
 * taken it away.
 *
 * Ailment 0x12 is the one that does: the actor goes plain white with its
 * shadow hidden, and the fade is left at 0xFF so the colour arrives in a
 * single frame rather than being walked toward. Everything else, no ailment
 * included, goes back to white at the ordinary rate.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* The two ailments that colour an actor. The one that lifts it off the floor
   is BTL_STATUS_LIFTED, beside the rest of them in actor.h. */
#define BTL_AIL_TINT_A 4
#define BTL_AIL_TINT_B 5

/* Plain white, and the two rates the colour is reached at. */
#define BTL_TINT_WHITE 0x80
#define BTL_TINT_NOW   0xFF
#define BTL_TINT_WALK  8

extern u_char g_btl_tint_ail4[];
extern u_char g_btl_tint_ail5[];

void BtlObjStatusTint(BtlObj *obj)
{
    int     red;
    union {
        BtlActor *actor;
        u_long mask;
    } work;

    obj->fade = BTL_TINT_NOW;
    obj->attr &= ~BTL_OBJ_NO_SHADOW;
    work.actor = obj->actor;
    switch ((signed char)work.actor->c.status) {
    case BTL_AIL_TINT_A: {
        BtlObj *shadow;
        red = g_btl_tint_ail4[0];
        shadow = obj->shadow;
        obj->rgb_to[0] = red;
        obj->rgb_to[1] = g_btl_tint_ail4[1];
        obj->rgb_to[2] = g_btl_tint_ail4[2];
        work.mask = ~BTL_OBJ_HIDDEN;
        shadow->attr &= work.mask;
        break;
    }
    case BTL_AIL_TINT_B: {
        BtlObj *shadow;
        red = g_btl_tint_ail5[0];
        shadow = obj->shadow;
        obj->rgb_to[0] = red;
        obj->rgb_to[1] = g_btl_tint_ail5[1];
        obj->rgb_to[2] = g_btl_tint_ail5[2];
        work.mask = ~BTL_OBJ_HIDDEN;
        shadow->attr &= work.mask;
        break;
    }
    case BTL_STATUS_LIFTED:
        /* The original rereads the flags after the initial shadow clear. */
        obj->attr = *(volatile u_long *)&obj->attr | BTL_OBJ_NO_SHADOW;
        obj->shadow->attr |= BTL_OBJ_HIDDEN;
        obj->rgb_to[0] = BTL_TINT_WHITE;
        obj->rgb_to[1] = BTL_TINT_WHITE;
        obj->rgb_to[2] = BTL_TINT_WHITE;
        return;
    default:
        obj->attr &= ~BTL_OBJ_NO_SHADOW;
        obj->shadow->attr &= ~BTL_OBJ_HIDDEN;
        obj->rgb_to[0] = BTL_TINT_WHITE;
        obj->rgb_to[1] = BTL_TINT_WHITE;
        obj->rgb_to[2] = BTL_TINT_WHITE;
        obj->fade = BTL_TINT_WALK;
        break;
    }
}

