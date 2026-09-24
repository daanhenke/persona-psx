/* Persona 1 (JP) - the room's actors built from the scene pack.  ADV only.
 *   ADV 0x80083F6C
 *
 * Four tables of definitions in the pack fill four stretches of the actor
 * records. The room's own eight actors, and the eight extras after them, each
 * come in two forms, and the event flag a definition names picks the second
 * once it is set - which is how a room changes as the story moves on. The two
 * remaining tables only place things: eight marks and eight more records,
 * both at full brightness. The party leader's record is emptied in between.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/adv/actor.h>
#include <persona/adv/scene.h>
#include <persona/common/eventflag.h>

#define PROP_ACTOR  16
#define MARK_ACTOR  8
#define SPOT_ACTOR  25
#define LEADER      24
#define FULL_BRIGHT 0x80
#define FLAG_MASK   0x1FF

#define FLAG_SET(id)                                                           \
    ((u_char)(g_event_flags[(short)((id) & FLAG_MASK) / 8] & (1 << ((id) & 7))))

/* 81.84%: the image loads each form's shadow as a halfword before storing
   its low byte, where gcc narrows the load to a byte, and the stores around
   it schedule differently as a result. */
#ifdef NON_MATCHING
void AdvBuildActors(void)
{
    AdvActorDef *d;
    AdvPropDef  *p;
    u_char       i;

    for (i = 0; i < ROOM_ACTORS; i++) {
        d = &g_room_actor_defs[g_adv_room][i];
        if (d->flag != 0xFFFF && FLAG_SET(d->flag)) {
            g_adv_actors[i].script = d->form[1].script;
            g_adv_actors[i].x = d->form[1].x;
            g_adv_actors[i].y = d->form[1].y;
            g_adv_actors[i].home_x = d->form[1].x;
            g_adv_actors[i].home_y = d->form[1].y;
            g_adv_actors[i].dir = d->form[1].u.b.dir & 3;
            g_adv_actors[i].next_dir = d->form[1].u.b.dir & 3;
            g_adv_actors[i].phase = 0;
            g_adv_actors[i].shadow = d->form[1].u.b.shadow;
            g_adv_actors[i].unk25 = 0;
            g_adv_actors[i].unk22 = d->form[1].unk22;
            g_adv_actors[i].unk26 = d->form[1].lift;
            g_adv_actors[i].bright = d->form[1].bright;
            g_adv_actors[i].unk08 = -1;
            g_adv_actors[i].flags = d->form[1].u.flags & 0xFFF0;
        } else {
            g_adv_actors[i].script = d->form[0].script;
            g_adv_actors[i].x = d->form[0].x;
            g_adv_actors[i].y = d->form[0].y;
            g_adv_actors[i].dir = d->form[0].u.b.dir & 3;
            g_adv_actors[i].next_dir = d->form[0].u.b.dir & 3;
            g_adv_actors[i].phase = 0;
            g_adv_actors[i].shadow = d->form[0].u.b.shadow;
            g_adv_actors[i].unk25 = 0;
            g_adv_actors[i].unk22 = d->form[0].unk22;
            g_adv_actors[i].unk26 = d->form[0].lift;
            g_adv_actors[i].bright = d->form[0].bright;
            g_adv_actors[i].unk08 = -1;
            g_adv_actors[i].flags = d->form[0].u.flags & 0xFFF0;
        }
    }

    for (i = 0; i < ROOM_ACTORS; i++) {
        p = &g_prop_defs[i];
        if (p->flag != 0xFFFF && FLAG_SET(p->flag)) {
            g_adv_actors[PROP_ACTOR + i].script = g_prop_defs[i].form[1].script;
            g_adv_actors[PROP_ACTOR + i].x = g_prop_defs[i].form[1].x;
            g_adv_actors[PROP_ACTOR + i].y = g_prop_defs[i].form[1].y;
            g_adv_actors[PROP_ACTOR + i].unk23 = g_prop_defs[i].form[1].unk23;
            g_adv_actors[PROP_ACTOR + i].unk22 = g_prop_defs[i].form[1].unk22;
            g_adv_actors[PROP_ACTOR + i].unk24 = 0;
            g_adv_actors[PROP_ACTOR + i].unk26 = g_prop_defs[i].form[1].lift;
            g_adv_actors[PROP_ACTOR + i].unk08 = -1;
            g_adv_actors[PROP_ACTOR + i].bright = g_prop_defs[i].form[1].bright;
            g_adv_actors[PROP_ACTOR + i].flags =
                g_prop_defs[i].form[1].unk23 & 0xFFF0;
        } else {
            g_adv_actors[PROP_ACTOR + i].script = p->form[0].script;
            g_adv_actors[PROP_ACTOR + i].x = p->form[0].x;
            g_adv_actors[PROP_ACTOR + i].y = p->form[0].y;
            g_adv_actors[PROP_ACTOR + i].unk23 = p->form[0].unk23;
            g_adv_actors[PROP_ACTOR + i].unk22 = p->form[0].unk22;
            g_adv_actors[PROP_ACTOR + i].unk24 = 0;
            g_adv_actors[PROP_ACTOR + i].unk26 = p->form[0].lift;
            g_adv_actors[PROP_ACTOR + i].unk08 = -1;
            g_adv_actors[PROP_ACTOR + i].bright = p->form[0].bright;
            g_adv_actors[PROP_ACTOR + i].flags = p->form[0].unk23 & 0xFFF0;
        }
    }

    for (i = 0; i < ROOM_ACTORS; i++) {
        g_adv_actors[MARK_ACTOR + i].x = g_mark_defs[i].x;
        g_adv_actors[MARK_ACTOR + i].y = g_mark_defs[i].y;
        g_adv_actors[MARK_ACTOR + i].bright = FULL_BRIGHT;
        g_adv_actors[MARK_ACTOR + i].unk26 = g_mark_defs[i].lift;
    }

    g_adv_actors[LEADER].script = -1;
    g_adv_actors[LEADER].x = 0;
    g_adv_actors[LEADER].y = 0;
    g_adv_actors[LEADER].unk23 = 0;
    g_adv_actors[LEADER].unk22 = 0;
    g_adv_actors[LEADER].unk24 = 0;
    g_adv_actors[LEADER].bright = FULL_BRIGHT;
    g_adv_actors[LEADER].unk08 = -1;
    for (i = 0; i < ROOM_ACTORS; i++) {
        g_adv_actors[SPOT_ACTOR + i].x = g_spot_defs[i].x;
        g_adv_actors[SPOT_ACTOR + i].y = g_spot_defs[i].y;
        g_adv_actors[SPOT_ACTOR + i].dir = g_spot_defs[i].dir;
        g_adv_actors[SPOT_ACTOR + i].bright = FULL_BRIGHT;
        g_adv_actors[SPOT_ACTOR + i].unk26 = g_spot_defs[i].lift;
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/game/actorsbuild", AdvBuildActors);
#endif
