/* Persona 1 (JP) - the two scenes that interrupt a scripted fight.  BTLP only.
 *   0x80099908 BtlRoundOverScene   0x80099EA8 BtlLastEnemyScene
 *
 * Both hang off the end of the round and both do nothing at all unless this
 * is their own encounter and the fighter they watch is down.
 *
 * BtlRoundOverScene is the one that changes shape. Once the first enemy of
 * encounter 15 has fallen and the second is still standing, it pulls the
 * markers in, plays four lines, sinks the model out of the field, whitens its
 * palette, shrinks it to nothing, puts the second form's script on it, brings
 * it back up at full size with the palette it started with, and hands the
 * grid square and the key over to the second record. Four more lines follow
 * and the round is sent to its ninth step. A latch of its own keeps all of
 * that to once a battle.
 *
 * BtlLastEnemyScene is the plain version for encounter 16: markers in, four
 * lines, one script, no palette work.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <libsnd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>

/* The encounters each scene belongs to. */
#define SCENE_ROUND_OVER_ENCOUNTER 0xF
#define SCENE_LAST_ENEMY_ENCOUNTER 0x10

/* The slot the scripted scenes' sound is opened in, and the four noises the
   change of shape makes. */
#define SCENE_SOUND_SLOT 3
#define SCENE_SE_ROAR    3
#define SCENE_SE_SINK    0
#define SCENE_SE_SHRINK  1
#define SCENE_SE_GROW    2

/* The pack banks the two scenes play out of. */
#define SCENE_MORPH_BANK 0xC4
#define SCENE_LAST_BANK  0xC3

/* Frames between one line and the next. */
#define SCENE_SETTLE 60

/* Which script of the model's table each step arms. */
#define SCENE_SCRIPT_SINK   0xE
#define SCENE_SCRIPT_SECOND 0xD
#define SCENE_SCRIPT_LAST   0x12

/* Whoever speaks each line. */
#define SCENE_VOICE_BOSS   0x27
#define SCENE_VOICE_NOBODY 0

/* An actor's palette: 256 entries, one page each. */
#define BTL_CLUT_ENTRIES 0x100
#define BTL_CLUT_BYTES   0x200
#define BTL_CLUT_WHITE   0xFFFF

/* Set on the model as the shape changes; what the drawing side makes of it is
   not established. */
#define BTL_OBJ_ATTR_4000 0x4000

/* How far the model sinks each frame and how far down it goes, then how much
   of the gap the shrink and the growth close each frame and where each of
   them stops. */
#define SCENE_SINK_STEP  (-0x8000)
#define SCENE_SINK_FLOOR (-0x500000)
#define SCENE_SCALE_STEP 32
#define SCENE_SCALE_GONE 0x20
#define SCENE_SCALE_FULL 0x1000

/* Where the second form comes up, and the row and grid code it takes. */
#define SCENE_SECOND_Y   0xFFC40000
#define SCENE_SECOND_ROW 4
#define SCENE_SECOND_KEY 0xB9
#define BTL_GRID_WIDTH   9

/* The step BtlStageRound is sent to once the shape has changed. */
#define BTL_STEP_MORPHED 9

/* Raised the first time the scene plays, so a battle only sees it once. */
extern u_char g_btl_round_over_done;

extern u_short g_btl_clut_fading;
extern u_char *g_btl_actor_clut;
extern u_char *g_btl_actor_clut_to;
extern u_char *g_btl_actor_clut_base;
extern u_char  g_btl_grid[];

extern u_char g_btl_line_enc15a[];
extern u_char g_btl_line_enc15b[];
extern u_char g_btl_line_enc15c[];
extern u_char g_btl_line_enc15d[];
extern u_char g_btl_line_enc15e[];
extern u_char g_btl_line_enc15f[];
extern u_char g_btl_line_enc15g[];
extern u_char g_btl_line_enc15h[];
extern u_char g_btl_line_enc16a[];
extern u_char g_btl_line_enc16b[];
extern u_char g_btl_line_enc16c[];
extern u_char g_btl_line_enc16d[];


void BtlRoundOverScene(void)
{
    BtlObj *obj;
    int     i;
    int     n;

    if (g_btl_encounter == SCENE_ROUND_OVER_ENCOUNTER
        && g_btl_enemies[0].c.hp == 0 && g_btl_enemies[1].c.hp != 0
        && g_btl_round_over_done == 0) {
        g_btl_round_over_done = 1;

        while (!BtlMarkersHidden()) {
            BtlDrawFrame();
        }
        BtlRetractMarkers();
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);

        BtlPlayScene(SCENE_VOICE_BOSS, g_btl_line_enc15a);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);

        BtlLoadPackBank(SCENE_MORPH_BANK);
        BtlSePlay(SCENE_SOUND_SLOT, SCENE_SE_ROAR);
        BtlPlayScene(SCENE_VOICE_BOSS, g_btl_line_enc15b);

        obj = g_btl_enemies[0].obj;
        i = 0;
        BtlObjSetScript(obj, obj->scripts[SCENE_SCRIPT_SINK]);
        BtlSePlay(SCENE_SOUND_SLOT, SCENE_SE_SINK);
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);

        do {
            obj->z += SCENE_SINK_STEP;
            BtlDrawFrame();
        } while (obj->z > SCENE_SINK_FLOOR);

        BtlPlayScene(SCENE_VOICE_NOBODY, g_btl_line_enc15c);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);
        BtlPlayScene(SCENE_VOICE_BOSS, g_btl_line_enc15d);

        /* Its own counter, not the one the frame waits use. */
        for (n = 1; n < BTL_CLUT_ENTRIES; n++) {
            ((u_short *)g_btl_actor_clut_to)[obj->mark_num * BTL_CLUT_ENTRIES
                                             + n] = BTL_CLUT_WHITE;
        }
        g_btl_clut_fading |= 1 << obj->mark_num;

        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);

        BtlSePlay(SCENE_SOUND_SLOT, SCENE_SE_SHRINK);
        SsSepStop(g_btl_seq[SCENE_SOUND_SLOT], SCENE_SOUND_SLOT);
        obj->attr |= BTL_OBJ_ATTR_4000;
        do {
            obj->scale_x -= obj->scale_x / SCENE_SCALE_STEP;
            obj->scale_y -= obj->scale_y / SCENE_SCALE_STEP;
            BtlDrawFrame();
        } while (obj->scale_x >= SCENE_SCALE_GONE + 1);

        obj->scale_x = SCENE_SCALE_GONE;
        obj->scale_y = SCENE_SCALE_GONE;
        obj->z = 0;
        BtlObjSetScript(obj, obj->scripts[SCENE_SCRIPT_SECOND]);
        obj->shadow->attr |= BTL_OBJ_HIDDEN;
        obj->scale_x = SCENE_SCALE_GONE;
        obj->scale_y = SCENE_SCALE_GONE;
        obj->attr |= BTL_OBJ_ATTR_4000;
        obj->z += (int)0xFF9C0000;
        memcpy(g_btl_actor_clut + obj->mark_num * BTL_CLUT_BYTES,
               g_btl_actor_clut_base + obj->mark_num * BTL_CLUT_BYTES,
               BTL_CLUT_BYTES);
        BtlSePlay(SCENE_SOUND_SLOT, SCENE_SE_GROW);

        do {
            obj->scale_x += obj->scale_x / SCENE_SCALE_STEP;
            obj->scale_y += obj->scale_y / SCENE_SCALE_STEP;
            BtlDrawFrame();
        } while (obj->scale_x < SCENE_SCALE_FULL);

        g_btl_actors[obj->mark_num].c.key = 0;
        obj->attr |= BTL_OBJ_HIDDEN;
        g_btl_grid[obj->row * BTL_GRID_WIDTH + obj->col2] = SCENE_SECOND_KEY;
        g_btl_enemies[1].c.key = SCENE_SECOND_KEY;
        g_btl_enemies[1].obj->attr &= ~BTL_OBJ_HIDDEN;
        g_btl_enemies[1].obj->shadow->attr &= ~BTL_OBJ_HIDDEN;
        g_btl_enemies[1].obj->row = SCENE_SECOND_ROW;
        g_btl_enemies[1].obj->y = SCENE_SECOND_Y;
        g_btl_enemies[1].obj->y2 = SCENE_SECOND_Y;
        obj->scale_x = SCENE_SCALE_FULL;
        obj->scale_y = SCENE_SCALE_FULL;

        BtlPlayScene(2, g_btl_line_enc15e);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);
        BtlPlayScene(5, g_btl_line_enc15f);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);
        BtlPlayScene(4, g_btl_line_enc15g);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);
        BtlPlayScene(SCENE_VOICE_BOSS, g_btl_line_enc15h);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);

        BtlSoundClose(SCENE_SOUND_SLOT);
        BtlRefreshMarkers();
        g_btl_talk_outcome = 0;
        g_btl_step = BTL_STEP_MORPHED;
    }
}

void BtlLastEnemyScene(void)
{
    BtlObj *obj;
    int     i;

    if (g_btl_encounter == SCENE_LAST_ENEMY_ENCOUNTER
        && g_btl_enemies[0].c.hp == 0) {
        while (!BtlMarkersHidden()) {
            BtlDrawFrame();
        }
        BtlRetractMarkers();
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);

        BtlPlayScene(SCENE_VOICE_NOBODY, g_btl_line_enc16a);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);

        BtlLoadPackBank(SCENE_LAST_BANK);
        obj = g_btl_enemies[0].obj;
        BtlObjSetScript(obj, obj->scripts[SCENE_SCRIPT_LAST]);
        while (obj->attr & BTL_OBJ_ANIMATING) {
            BtlDrawFrame();
        }

        BtlPlayScene(5, g_btl_line_enc16b);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);
        BtlPlayScene(2, g_btl_line_enc16c);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);
        BtlPlayScene(SCENE_VOICE_NOBODY, g_btl_line_enc16d);
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < SCENE_SETTLE);

        BtlSoundClose(SCENE_SOUND_SLOT);
    }
}
