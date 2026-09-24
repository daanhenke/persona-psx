/* Persona 1 (JP) - the party walking out through an exit.  ADV only.
 *   0x8008060C AdvExitPlaceLeader
 *
 * An exit the scene pack gives a walk-out sprite has the leader drawn leaving
 * by it: the exit's sound plays, and record 24 is put on the exit's walk-out
 * tile, behind everything but the backdrop, with the sprite for that exit.
 * An exit left only upwards takes the second of the pair.
 */
#include <decomp/types.h>
#include <persona/adv/actor.h>
#include <persona/adv/scene.h>

#define LEADER      24
#define LEADER_SLOT 0x32
#define LEAVE_NONE  0xFF
#define LEAVE_Z     0x3BD

extern void *g_leave_defs[];

extern void AdvSoundCommand(short cmd);
extern void ActorSetTile(short x, short y, AdvActor *a);
extern void SlotInit(void *def, u_char slot, int attr, short x, short y);

void AdvExitPlaceLeader(u_char n)
{
    u_char def;

    if (g_adv_scene->entries[n].leave != LEAVE_NONE) {
        AdvSoundCommand(g_adv_scene->exit_sound);
        ActorSetTile(g_adv_scene->entries[n].leave_x,
                     g_adv_scene->entries[n].leave_y, &g_adv_actors[LEADER]);
        g_adv_actors[LEADER].z = LEAVE_Z;
        g_adv_actors[LEADER].world_y += 8;
        g_adv_actors[LEADER].id = 0;
        def = g_adv_scene->entries[n].leave * 2;
        if (g_adv_scene->entries[n].dirs == 1) {
            def |= 1;
        }
        SlotInit(g_leave_defs[def], LEADER_SLOT, LEAVE_Z,
                 g_adv_actors[LEADER].world_x, g_adv_actors[LEADER].world_y);
    }
}
