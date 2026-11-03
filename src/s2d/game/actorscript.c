#include <decomp/types.h>

typedef struct {
    u8 colour, enabled, pad2[2];
    union {
        u16 packed;
        u8 xy[2];
    } pos;
    u16 previous;
    u8 speed, fracX, fracY;
    s8 dx, dy;
    u8 direction, pad14[2], axis, turn, blocked, pad19;
    u8 *script, *restart, *loop;
    u8 repeat, pad33;
    s16 timer;
} MapActor;
typedef struct {
    u8 before[9];
    s8 direction;
    u8 after[26];
} LookupRecord;
extern LookupRecord D_800B0FB0[];
extern u8 D_800B1D30[], *D_800A514C[];
extern s8 D_80064FF0[2][4];
extern int rand(void), func_80094F9C(u16, u16);
extern int func_80094AA4(s16, s16);
extern void func_80094E68(MapActor *, s16, s16 *);
#ifndef PERSONA_TILE_MAP
#define PERSONA_TILE_MAP ((u8 *)0x80122000)
#endif

/* Advance an actor command stream, then handle one tile crossing. */
s32 func_80095700(MapActor *actor) {
    s16 coords[2];
    int i, run = 1, found;
    u8 *ptr, *stepPtr;
    u16 tile, tile2;
    if (actor->blocked) {
        i = 3;
        ptr = PERSONA_TILE_MAP;
        for (; i > 0; i--) {
            func_80094E68(actor, i, coords);
            if (*(u16 *)((coords[0] * 2 + (coords[1] * 400 + *(s32 *)ptr)) + (unsigned long)ptr) &
                0x800)
                break;
        }
        if (i == 0) {
            actor->blocked = 0;
            actor->timer = rand() % 5 + 2;
            {
                actor->script = D_800A514C[5];
                actor->restart = actor->script;
            }
        }
    }
    while (run == 1) {
        ptr = &actor->fracY;
        switch (*actor->script) {
        case 0: {
            actor->script = D_800A514C[0];
            actor->restart = actor->script;
        } break;
        case 1:
            run = 0;
            actor->script++;
            break;
        case 2:
            actor->script = actor->restart;
            break;
        case 3:
            actor->script++;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            actor->speed = *actor->script - 4;
            actor->script++;
            break;
        case 9:
            actor->script++;
            actor->speed++;
            if ((s8)actor->speed >= 4) {
                actor->speed = 4;
                {
                    actor->script = D_800A514C[1];
                    actor->restart = actor->script;
                }
                run = 0;
            }
            break;
        case 10:
            actor->script++;
            actor->speed = (s8)actor->speed - 1;
            if ((s8)actor->speed < 0) {
                actor->script = D_800A514C[0];
                actor->restart = actor->script;
            }
            break;
        case 11:
            actor->script++;
            actor->turn--;
            break;
        case 12:
            actor->script++;
            actor->turn++;
            break;
        case 13:
            *(ptr - (actor->direction & 1)) = 0;
            actor->script++;
            actor->direction = (s8)actor->direction - 1;
            if ((s8)actor->direction < 0)
                actor->direction = 3;
            actor->turn = 0;
            break;
        case 14:
            *(ptr - (actor->direction & 1)) = 0;
            actor->script++;
            actor->direction++;
            if ((s8)actor->direction >= 4)
                actor->direction = 0;
            actor->turn = 0;
            break;
        case 15:
            if (D_800B1D30[actor->axis] == 1) {
                actor->timer = rand() % 2 + 1;
                {
                    actor->script = D_800A514C[5];
                    actor->restart = actor->script;
                }
            } else
                goto pause;
            break;
        case 16:
        case 17:
            stepPtr = (u8 *)actor + ((actor->direction & 1) + 9);
            i = *actor->script - 16;
            actor->script++;
            *stepPtr += *actor->script;
            stepPtr = (u8 *)actor + ((actor->direction & 1) + 11);
            *stepPtr = D_80064FF0[i][(s8)actor->direction];
            actor->script++;
            break;
        case 18:
            if (actor->timer) {
                actor->timer--;
                if (actor->timer == 0)
                    actor->script += 2;
                run = 0;
            } else {
                actor->timer = actor->script[1];
            pause:
                run = 0;
            }
            break;
        case 19:
            actor->script++;
            {
                actor->script = D_800A514C[*actor->script];
                actor->restart = actor->script;
            }
            break;
        case 20:
            actor->repeat = *++actor->script;
            actor->loop = ++actor->script;
            break;
        case 21:
            actor->repeat = (s8)actor->repeat - 1;
            if (actor->repeat)
                actor->script = actor->loop;
            else
                actor->script++;
            break;
        }
    }
    stepPtr = (u8 *)actor - ((actor->direction & 1) - 10);
    *stepPtr += actor->speed;
    if ((s8)*stepPtr >= 16) {
        *stepPtr = (s8)*stepPtr - 16;
        actor->previous = actor->pos.packed;
        func_80094E68(actor, 1, coords);
        actor->pos.xy[0] = coords[0];
        actor->pos.xy[1] = coords[1];
        if (!actor->blocked) {
            i = 3;
            ptr = PERSONA_TILE_MAP;
            for (; i > 0; i--) {
                func_80094E68(actor, i, coords);
                if ((*(u16 *)((coords[0] * 2 + (coords[1] * 400 + *(s32 *)ptr)) +
                              (unsigned long)ptr) &
                     0x800) &&
                    (found = func_80094F9C(coords[0], coords[1])) != -1 &&
                    D_800B0FB0[found].direction == (s8)actor->direction) {
                    actor->blocked = 1;
                    actor->script = D_800A514C[2];
                    actor->restart = actor->script;
                    return 1;
                }
            }
        }
        func_80094E68(actor, 2, coords);
        tile = *(u16 *)(PERSONA_TILE_MAP + coords[0] * 2 + (coords[1] * 400 + *(s32 *)PERSONA_TILE_MAP));
        if ((tile & 0x2000) && actor->axis == 0 && !(tile & 0x1000) && D_800B1D30[0] == 0) {
            actor->script = D_800A514C[3];
            actor->restart = actor->script;
            return 1;
        }
        func_80094E68(actor, 2, coords);
        tile2 = *(u16 *)(PERSONA_TILE_MAP + coords[0] * 2 + (coords[1] * 400 + *(s32 *)PERSONA_TILE_MAP));
        if ((tile2 & 0x4000) && actor->axis == 1 && !(tile2 & 0x1000) && D_800B1D30[1] == 0) {
            actor->script = D_800A514C[3];
            actor->restart = actor->script;
            return 1;
        }
        func_80094E68(actor, 1, coords);
        if ((u8)func_80094AA4(coords[0], coords[1]) == 1) {
            actor->script = D_800A514C[6];
            actor->restart = actor->script;
            return 1;
        }
        return 1;
    }
    return 0;
}
