/* Persona 1 (JP) - the three openings a scripted fight can have.  BTLP only.
 *   0x80099500 BtlOpenEnemyEntrance
 *   0x800995B4 BtlOpenEnemyRise
 *   0x8009967C BtlOpenEnemyWhiten
 *
 * BtlStageOpen runs all three in turn and each belongs to one encounter or
 * one pair of them. Everything they do is done to the first enemy record,
 * which in a scripted fight is the only one that matters.
 *
 * The whitening is the longest and the only one that touches the palettes:
 * the fighter's target palette is filled with white, the fade is switched on
 * for its page, and a second later the original is copied back over the
 * target so the colour walks home again while the model grows from a quarter
 * size to full.
 *
 * Two of the three belong to a pair of encounters and test for it as one
 * unsigned range - the cast is what puts that on a single sltiu, and reading
 * the encounter unsigned is what makes the load lhu. The third wants one
 * value and reads it signed, which is the declaration every other unit has.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>

/* The slot the scripted scenes' sound is opened in. */
#define SCENE_SOUND_SLOT 3

/* Frames each step is left to settle. */
#define OPEN_SETTLE      60
#define OPEN_SETTLE_LONG 120

/* Which script of the model's table each step arms. */
#define OPEN_SCRIPT_ENTRANCE 1
#define OPEN_SCRIPT_RISE     0x11
#define OPEN_SCRIPT_WHITEN   0x19
#define OPEN_SCRIPT_SETTLED  0x1A

/* Which encounters each step belongs to, and the pack bank the entrance
   plays out of. */
#define OPEN_ENTRANCE_FIRST 0x11
#define OPEN_ENTRANCE_COUNT 2
#define OPEN_RISE_FIRST     0xA
#define OPEN_RISE_COUNT     2
#define OPEN_WHITEN         0x1F
#define OPEN_ENTRANCE_BANK  0xB4

/* Set on an object by a script step that has left the ground; the rise waits
   for it to be the only half of the pair still standing. */
#define BTL_OBJ_JUMPED 0x08000000

/* Set on the model as the whitening starts and again as the round-over scene
   does; what the drawing side makes of it is not established. */
#define BTL_OBJ_ATTR_4000 0x4000

/* An actor's palette: 256 entries, one page each. */
#define BTL_CLUT_ENTRIES 0x100
#define BTL_CLUT_BYTES   0x200
#define BTL_CLUT_WHITE   0xFFFF

/* Where the model is scaled to, and how much of the gap it closes each
   frame. */
#define OPEN_SCALE_SMALL 0x400
#define OPEN_SCALE_FULL  0x1000
#define OPEN_SCALE_STEP  32

extern u_short  g_btl_clut_fading;
extern u_char  *g_btl_actor_clut_to;
extern u_char  *g_btl_actor_clut_base;


void BtlOpenEnemyEntrance(void)
{
    BtlObj *obj;
    int     i;

    obj = g_btl_enemies[0].obj;
    if ((u_int)((u_short)g_btl_encounter - OPEN_ENTRANCE_FIRST)
        < OPEN_ENTRANCE_COUNT) {
        BtlLoadPackBank(OPEN_ENTRANCE_BANK);
        BtlObjSetScript(obj, obj->scripts[OPEN_SCRIPT_ENTRANCE]);
        while (obj->attr & BTL_OBJ_ANIMATING) {
            BtlDrawFrame();
        }
        BtlSoundClose(SCENE_SOUND_SLOT);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < OPEN_SETTLE);
    }
}

#ifdef NON_MATCHING
void BtlOpenEnemyRise(void)
{
    BtlObj *obj;
    int     i;

    obj = g_btl_enemies[0].obj;
    if ((u_int)((u_short)g_btl_encounter - OPEN_RISE_FIRST)
        < OPEN_RISE_COUNT) {
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < OPEN_SETTLE_LONG);
        BtlObjSetScript(obj, obj->scripts[OPEN_SCRIPT_RISE]);
        /* One instruction over: gcc fills the guard's delay slot with the
           counter's zero and then re-materialises it, where the image puts
           the loop's own constant there. Nothing at the source level has
           moved it - the loop shape, the operand order and a second counter
           were all tried. */
        while ((obj->attr & BTL_OBJ_BUSY_MASK) != BTL_OBJ_JUMPED) {
            BtlDrawFrame();
        }
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < OPEN_SETTLE);
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/openscenes", BtlOpenEnemyRise);
#endif

void BtlOpenEnemyWhiten(void)
{
    BtlObj *obj;
    int     i;
    int     n;

    obj = g_btl_enemies[0].obj;
    if (g_btl_encounter == OPEN_WHITEN) {
        while (obj->attr & BTL_OBJ_ANIMATING) {
            BtlDrawFrame();
        }

        /* Its own counter, not the one the frame waits use: the image keeps
           it in a temporary, which only happens if it is a second variable. */
        for (n = 1; n < BTL_CLUT_ENTRIES; n++) {
            ((u_short *)g_btl_actor_clut_to)[obj->mark_num * BTL_CLUT_ENTRIES
                                             + n] = BTL_CLUT_WHITE;
        }
        g_btl_clut_fading |= 1 << obj->mark_num;

        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < OPEN_SETTLE);

        obj->attr |= BTL_OBJ_ATTR_4000;
        obj->shadow->attr |= BTL_OBJ_HIDDEN;
        obj->scale_x = OPEN_SCALE_SMALL;
        obj->scale_y = OPEN_SCALE_SMALL;
        BtlObjSetScript(obj, obj->scripts[OPEN_SCRIPT_WHITEN]);

        memcpy(g_btl_actor_clut_to + obj->mark_num * BTL_CLUT_BYTES,
               g_btl_actor_clut_base + obj->mark_num * BTL_CLUT_BYTES,
               BTL_CLUT_BYTES);
        g_btl_clut_fading |= 1 << obj->mark_num;

        do {
            obj->scale_x += obj->scale_x / OPEN_SCALE_STEP;
            obj->scale_y += obj->scale_y / OPEN_SCALE_STEP;
            BtlDrawFrame();
        } while (obj->scale_x < OPEN_SCALE_FULL);

        obj->scale_x = OPEN_SCALE_FULL;
        obj->scale_y = OPEN_SCALE_FULL;
        obj->shadow->attr &= ~BTL_OBJ_HIDDEN;

        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < OPEN_SETTLE_LONG);
        BtlObjSetScript(obj, obj->scripts[OPEN_SCRIPT_SETTLED]);
    }
}
