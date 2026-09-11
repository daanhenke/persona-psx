/* Persona 1 (JP) - putting the field back once the fight is over.  BTLP only.
 *   0x80098834 BtlRestoreField
 *
 * The last of the three BtlStageClose runs after the outcome, and it only
 * happens if a party member went down during the fight and was not set aside
 * on purpose. The caller has already cleared their ailments and put them back
 * on one hit point; this is the part the player sees.
 *
 * The intro object is the whole screen, so fading its colour up to white and
 * back down again is the flash everything else happens behind: whoever is
 * still standing on the enemy side is taken out of the drawing pass and their
 * record emptied, every member who can stand is put back in their square at
 * full brightness, and the members that were set aside are brought back into
 * the fight by clearing both of their flags at once.
 *
 * Everything here reaches an actor's object through the record each time
 * rather than holding it in a local: a store through the pointer could be a
 * store to it, so the image reloads before each one and so does this. The
 * flash is the exception - its colours go through a local and only its
 * attribute through the global, which is the second of the two loads the
 * image makes.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/status.h>

/* Set on a member who has been taken out of the fight rather than knocked
   out of it; the sweep at the end puts them back by clearing it and the
   out-of-fight flag together. */
#define BTL_ACTOR_ASIDE 0x80000000

/* The page the flash is drawn out of, how fast it walks, and the two colours
   it walks between. */
#define FLASH_TPAGE 0x40
#define FLASH_FADE  8
#define FLASH_WHITE 0xFF

/* What a member is put back at. */
#define MEMBER_LIT 0x80

/* Cleared off a member's object as it goes back, and off its shadow. */
#define MEMBER_CLEAR (BTL_OBJ_HIDDEN | BTL_OBJ_NO_SHADOW)

extern void BtlPickSettle(void);
extern void BtlPlaceMember(int slot, short col, short row);
extern void func_800A735C(void);


void BtlRestoreField(void)
{
    BtlObj   *flash;
    int       found;
    int       i;

    /* The counter is set up before the answer. gcc derives two walkers of its
       own from the index - one biased to the flag word and one a plain byte
       offset - and which register each gets follows the order these two are
       written in; the other way round swaps them for the whole loop. */
    i = 0;
    found = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && ((signed char)g_btl_actors[i].c.status == BTL_STATUS_DOWN
                || (g_btl_actors[i].flags & BTL_ACTOR_OUT) != 0)
            && (g_btl_actors[i].flags & BTL_ACTOR_ASIDE) == 0) {
            found = 1;
            break;
        }
        i++;
    } while (i < BTL_PARTY);
    if (found == 0) {
        return;
    }

    /* The colours go through a local and the attribute through the global:
       the image loads the pointer twice, and a store through it could be a
       store to it, so the second load is the one the attribute uses. */
    g_btl_tpage[0] = FLASH_TPAGE;
    flash = g_btl_intro_obj;
    flash->fade = FLASH_FADE;
    flash->rgb[0] = 0;
    flash->rgb[1] = 0;
    flash->rgb[2] = 0;
    flash->rgb_to[0] = FLASH_WHITE;
    flash->rgb_to[1] = FLASH_WHITE;
    flash->rgb_to[2] = FLASH_WHITE;
    g_btl_intro_obj->attr &= ~BTL_OBJ_HIDDEN;
    while (g_btl_intro_obj->rgb[0] != FLASH_WHITE) {
        BtlDrawFrame();
    }

    g_btl_intro_obj->attr |= BTL_OBJ_HIDDEN;
    for (i = BTL_PARTY; i < BTL_ACTORS; i++) {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            g_btl_actors[i].obj->attr |= BTL_OBJ_HIDDEN;
            g_btl_actors[i].obj->shadow->attr |= BTL_OBJ_HIDDEN;
            g_btl_actors[i].obj->mark->attr |= BTL_OBJ_HIDDEN;
            g_btl_actors[i].obj->mark->attached->attr |= BTL_OBJ_HIDDEN;
            g_btl_actors[i].c.key = 0;
        }
    }

    for (i = 0; i < BTL_PARTY; i++) {
        if ((g_btl_actors[i].flags & BTL_ACTOR_ASIDE) != 0) {
            g_btl_actors[i].flags &= ~(BTL_ACTOR_ASIDE | BTL_ACTOR_OUT);
        }
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            BtlPlaceMember(i, g_btl_actors[i].obj->col2 >> 1,
                           g_btl_actors[i].obj->row);
            g_btl_actors[i].obj->rgb_to[0] = MEMBER_LIT;
            g_btl_actors[i].obj->rgb_to[1] = MEMBER_LIT;
            g_btl_actors[i].obj->rgb_to[2] = MEMBER_LIT;
            g_btl_actors[i].obj->rgb[0] = MEMBER_LIT;
            g_btl_actors[i].obj->rgb[1] = MEMBER_LIT;
            g_btl_actors[i].obj->rgb[2] = MEMBER_LIT;
            g_btl_actors[i].obj->attr &= ~MEMBER_CLEAR;
            g_btl_actors[i].obj->shadow->attr &= ~BTL_OBJ_HIDDEN;
        }
    }

    BtlPickSettle();
    BtlShowAilmentMarks(0);
    g_btl_intro_obj->rgb_to[0] = 0;
    g_btl_intro_obj->rgb_to[1] = 0;
    g_btl_intro_obj->rgb_to[2] = 0;
    g_btl_intro_obj->attr &= ~BTL_OBJ_HIDDEN;
    while (g_btl_intro_obj->rgb[0] != 0) {
        BtlDrawFrame();
    }
    func_800A735C();
    BtlRefreshMarkers();
}
