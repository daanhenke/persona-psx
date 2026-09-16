/* Persona 1 (JP) - calling a Persona out while the disc is read.  BTLP only.
 *   0x800853C8 BtlSummonPersona
 *
 * The same summon BtlSummonActorPersona makes, taken a step at a time so the
 * field keeps moving under it: four steps, one a frame, with a frame drawn at
 * the bottom of every pass and each waiting step doing nothing until the drive
 * is idle again.
 *
 * Step nought works out which Persona the acting fighter carries and puts its
 * graphics id on the fighter's own record, where the steps after it read it
 * back, and starts that file reading into the staging buffer. Step one reads
 * the summon's sound pack - one of two, by which of the two calls this is -
 * into the pack buffer. Step two opens the pack in the music slot, takes the
 * arena down to a quarter and the fighter up to full white on a one-step fade,
 * and plays the fanfare. Step three puts the Persona up where the fighter
 * stands and hands it back.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/cast.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stats.h>

/* Where the Persona's file is read to - the staging buffer the spawn takes
   the TIM and the model out of, as summonpersona.c has it. */
#define SUMMON_STAGE ((u_long *)0x80140000)

/* The two sound packs the summon comes with, and where one is read to. */
#define SUMMON_PACK0  0x10D
#define SUMMON_PACK1  0x10E
#define SUMMON_BUFFER ((u_long *)0x80152400)
#define SUMMON_SEPS   1

/* What the fighter is drawn at while its Persona arrives, and what the arena
   behind it drops to. */
#define SUMMON_ACTOR_RGB 0xFF
#define SUMMON_SCENE_RGB 0x40
#define SUMMON_FADE      1

/* Which bank the summon's sound comes out of. */
#define SUMMON_BANK 5

/* Attribute bits, the scale the Persona stands at, and the motion it opens
   with. */
#define SUMMON_ATTR     0x20000200
#define SUMMON_SCALE_XY 0x100
#define SUMMON_SCALE_Z  0x1000
#define SUMMON_MOTION   3

BtlObj *BtlSummonPersona(int actor, int which)
{
    CdlLOC       loc;
    BtlSoundBank bank;
    BtlObj      *obj;
    int          entry;
    int          step;

    step = 0;
    obj  = g_btl_actors[actor].obj;
    entry = SUMMON_PACK0;
    if (which != 0) {
        entry = SUMMON_PACK1;
    }
    while (1) {
        switch (step) {
        case 0:
            step = 1;
            obj->actor->summon =
                g_btl_personas[BtlActorPersona(obj->mark_num)].key;
            CdIntToPos(g_btl_persona_sectors[obj->actor->summon]
                           + g_btl_persona_gfx_base,
                       &loc);
            CdReadFileToAddrAsync(
                (CdlFILE *)&loc,
                g_btl_persona_sectors[obj->actor->summon + 1]
                    - g_btl_persona_sectors[obj->actor->summon],
                SUMMON_STAGE);
            break;

        case 1:
            if (g_cd_busy != -1) {
                break;
            }
            CdIntToPos(g_btl_pack_sectors[entry] + g_btl_voice_base, &loc);
            step = 2;
            CdReadFileToAddrAsync((CdlFILE *)&loc,
                                  g_btl_pack_sectors[entry + 1]
                                      - g_btl_pack_sectors[entry],
                                  SUMMON_BUFFER);
            break;

        case 2:
            if (g_cd_busy != -1) {
                break;
            }
            bank.nsep = SUMMON_SEPS;
            bank.vb   = g_btl_pack_vb;
            bank.vh   = g_btl_pack_vh;
            bank.seq  = g_btl_pack_seq;
            BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
            /* Each step raises the counter where its own work is done rather
               than as it starts; written first the scheduler leaves it above
               the call's own arguments. */
            step = 3;
            obj->rgb_to[0]     = SUMMON_ACTOR_RGB;
            obj->rgb_to[1]     = SUMMON_ACTOR_RGB;
            obj->rgb_to[2]     = SUMMON_ACTOR_RGB;
            g_btl_scene_rgb[0] = SUMMON_SCENE_RGB;
            g_btl_scene_rgb[1] = SUMMON_SCENE_RGB;
            g_btl_scene_rgb[2] = SUMMON_SCENE_RGB;
            obj->fade          = SUMMON_FADE;
            g_btl_arena_fade   = SUMMON_FADE;
            BtlSePlay(BTL_BGM_SLOT, 0);
            break;

        case 3:
            if (g_cd_busy != -1) {
                break;
            }
            g_btl_persona_obj = BtlSpawnPersona(obj->actor->summon, obj->col2,
                                                obj->row, 0);
            BtlObjSetAttr(g_btl_persona_obj, SUMMON_ATTR);
            BtlObjSetScale(g_btl_persona_obj, SUMMON_SCALE_XY,
                           SUMMON_SCALE_XY, SUMMON_SCALE_Z);
            BtlObjSetMotion(g_btl_persona_obj, SUMMON_MOTION);
            return g_btl_persona_obj;
        }
        BtlDrawFrame();
    }
}
