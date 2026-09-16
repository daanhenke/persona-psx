/* Persona 1 (JP) - the flash a fighter's action opens with.  BTLP only.
 *   0x80084F44 BtlLoadActorGfx
 *
 * Six steps, one a frame, with a frame drawn at the bottom of every pass -
 * so the field keeps moving while the disc is read. The steps run straight
 * through without a way of failing: each one raises the counter as it starts
 * its own work, and the two that wait on the drive simply do nothing until it
 * is idle again.
 *
 * What is read is one artwork file and one sound bank. The artwork goes to the
 * staging buffer, its first run is copied into the effect's own graphics area
 * and bound there, and its TIM is uploaded; the bank is read into the voice
 * buffer and opened in the music slot, which is what the fanfare comes out of.
 *
 * Two records are then put up over the fighter, both from the spare graphics
 * slot's script table and both a little in front of it. Step four waits for
 * the pair to come up to half white and turns them round; step five waits for
 * them to reach nothing, puts the fighter itself back to full white, takes the
 * pair down, holds the screen for ninety frames and gives the slot back.
 *
 * The answer is always nought, and no caller reads it.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <libcd.h>
#include <libsnd.h>
#include <persona/main/cd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/gfx.h>
#include <persona/btlp/load.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>

/* The steps, and where each one's file sits. */
#define LOAD_STEPS       6
#define LOAD_GFX_ENTRY   0x170
#define LOAD_BANK_ENTRY  0xFE

/* One sub-sequence in the bank, and the sound it plays. */
#define LOAD_BANK_SEPS 1
#define LOAD_BANK_SE   0

/* Where the effect's artwork is staged, and how much of the file goes there.
   It is bound as a kind-3 image under an index of its own. */
#define LOAD_FX_GFX   ((u_char *)0x801D9400)
#define LOAD_FX_BYTES 0x1000
#define LOAD_FX_KIND  3
#define LOAD_FX_INDEX 0xFE

/* Where its TIM goes. */
#define LOAD_TIM_PAGE 0x1A
#define LOAD_TIM_SLOT 0x10

/* The two records: group 2, drawn by handler 5, and lifted in front of the
   fighter they stand on. */
#define LOAD_GROUP   2
#define LOAD_DRAW    5
#define LOAD_Z_LIFT  0x700000

/* What the pair is walked between, and how fast once it turns round. */
#define LOAD_RGB_PEAK 0x80
#define LOAD_FADE_UP  1
#define LOAD_FADE_OUT 4

/* What the fighter is put back to, and how long the screen is held after. */
#define LOAD_ACTOR_RGB 0xFF
#define LOAD_HOLD      0x5A

/* Both records drift half a place toward the camera every frame. */
#define LOAD_Z_STEP 0x8000

extern BtlObjDef g_btl_load_def;

int BtlLoadActorGfx(int actor)
{
    CdlLOC       loc;
    long         pos[3];
    BtlSoundBank bank;
    BtlObj      *front;
    BtlObj      *back;
    int          step;
    int          i;

    step = 0;
    while (1) {
        switch (step) {
        case 0:
            CdIntToPos(g_btl_persona_sectors[LOAD_GFX_ENTRY]
                           + g_btl_move_gfx_base,
                       &loc);
            CdReadFileToAddrAsync((CdlFILE *)&loc,
                                  g_btl_persona_sectors[LOAD_GFX_ENTRY + 1]
                                      - g_btl_persona_sectors[LOAD_GFX_ENTRY],
                                  BTL_LOAD_STAGE);
            step++;
            break;

        case 1:
            if (g_cd_busy != -1) {
                break;
            }
            CdIntToPos(g_btl_pack_sectors[LOAD_BANK_ENTRY] + g_btl_voice_base,
                       &loc);
            CdReadFileToAddrAsync((CdlFILE *)&loc,
                                  g_btl_pack_sectors[LOAD_BANK_ENTRY + 1]
                                      - g_btl_pack_sectors[LOAD_BANK_ENTRY],
                                  (u_long *)BTL_VOICE_BUFFER);
            step++;
            break;

        case 2:
            if (g_cd_busy != -1) {
                break;
            }
            bank.nsep = LOAD_BANK_SEPS;
            bank.vb   = g_btl_voice_vb;
            bank.vh   = g_btl_voice_vh;
            bank.seq  = g_btl_voice_seq;
            BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
            step++;
            break;

        case 3:
            if (SsVabTransCompleted(0) == 0) {
                break;
            }
            BtlSePlay(BTL_BGM_SLOT, LOAD_BANK_SE);
            g_btl_fx_gfx = LOAD_FX_GFX;
            memcpy(LOAD_FX_GFX, g_load_stage_1, LOAD_FX_BYTES);
            BtlBindGfx(LOAD_FX_KIND, LOAD_FX_INDEX, &g_btl_fx_gfx);
            BtlUploadTim((u_long *)g_load_stage, LOAD_TIM_PAGE, LOAD_TIM_SLOT,
                         1, 0, 1);
            step++;
            pos[0] = g_btl_actors[actor].obj->x;
            pos[1] = g_btl_actors[actor].obj->y;
            pos[2] = g_btl_actors[actor].obj->z - LOAD_Z_LIFT;
            g_btl_load_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
            g_btl_load_def.attr = 1;
            front = BtlObjAlloc(&g_btl_load_def, LOAD_GROUP, 0, LOAD_DRAW, 0,
                                pos, LOAD_TIM_PAGE, LOAD_TIM_SLOT);
            front->fade   = LOAD_FADE_UP;
            front->rgb[0] = 0;
            front->rgb[1] = 0;
            front->rgb[2] = 0;
            g_btl_load_def.scripts = ((const u_long ***)g_btl_unused_gfx)[1];
            g_btl_load_def.attr = 1;
            back = BtlObjAlloc(&g_btl_load_def, LOAD_GROUP, 0, LOAD_DRAW, 0,
                               pos, LOAD_TIM_PAGE, LOAD_TIM_SLOT);
            back->fade   = LOAD_FADE_UP;
            back->rgb[0] = 0;
            back->rgb[1] = 0;
            back->rgb[2] = 0;
            break;

        case 4:
            if (front->rgb[0] == LOAD_RGB_PEAK) {
                step++;
                front->rgb_to[0] = 0;
                front->rgb_to[1] = 0;
                front->rgb_to[2] = 0;
                back->rgb_to[0]  = 0;
                back->rgb_to[1]  = 0;
                back->rgb_to[2]  = 0;
                front->fade      = LOAD_FADE_OUT;
                back->fade       = LOAD_FADE_OUT;
            }
            front->z += LOAD_Z_STEP;
            back->z += LOAD_Z_STEP;
            break;

        case 5:
            if (front->rgb[0] == 0) {
                g_btl_actors[actor].obj->rgb[0] = LOAD_ACTOR_RGB;
                g_btl_actors[actor].obj->rgb[1] = LOAD_ACTOR_RGB;
                g_btl_actors[actor].obj->rgb[2] = LOAD_ACTOR_RGB;
                g_btl_actors[actor].obj->fade = LOAD_FADE_UP;
                BtlObjFree(front);
                BtlObjFree(back);
                for (i = 0; i < LOAD_HOLD; i++) {
                    BtlDrawFrame();
                }
                BtlSoundClose(BTL_BGM_SLOT);
                return 0;
            }
            front->z += LOAD_Z_STEP;
            back->z += LOAD_Z_STEP;
            break;
        }
        BtlDrawFrame();
    }
}
