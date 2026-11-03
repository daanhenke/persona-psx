#include <decomp/types.h>
typedef struct
{
    signed char colours[3];
} ActorColours;
typedef struct
{
    u_char      colour, enabled, unused2[2];
    u_char      x, y, last_y, last_x;
    u_char      unused8[3];
    signed char dx, dy;
    u_char      direction, unused14[2], axis, unused17[3];
    void *      model, *current_model;
    u_int       unused28[2];
} MapActor;
extern ActorColours D_80064FD8;
extern MapActor     D_800B0FAC[96];
extern void*        actor_model __asm__("D_800A514C+4");
extern int          VSync(int), rand(void);
extern void         srand(unsigned int);
extern bool         func_80094F2C(void*, short);
#ifndef PERSONA_TILE_MAP
#define PERSONA_TILE_MAP ((u_char*)0x80122000)
#endif
#define TILE(x, y) (*(u_short*)(PERSONA_TILE_MAP + *(u_int*)PERSONA_TILE_MAP + (y) * 400 + (x) * 2))
void func_80094660(void)
{
    ActorColours colours     = D_80064FD8;
    int          directional = 0;
    int          i;
    MapActor*    actor = D_800B0FAC;
    srand(VSync(-1));
    for (i = 0; i < 96; i++)
    {
        int direction;
        actor->colour        = colours.colours[rand() % 3];
        actor->model         = actor_model;
        actor->current_model = actor_model;
        if (directional >= 24)
            direction = (rand() % 2) * 2 + 1;
        else
            direction = rand() % 4;
        actor->direction = direction;
        actor->enabled   = 1;
        switch (direction)
        {
            case 0:
                actor->x      = (rand() & 1) | 100;
                actor->last_x = actor->x;
                actor->y      = (rand() % 50) * 4;
                actor->last_y = actor->y;
                actor->dy     = 1;
                actor->axis   = 0;
                break;
            case 1:
                actor->x      = (rand() % 50) * 4;
                actor->last_y = actor->x;
                actor->y      = (rand() & 3) | 60;
                actor->last_x = actor->y;
                actor->dx     = 1;
                actor->axis   = 1;
                break;
            case 2:
                actor->x      = (rand() & 1) | 102;
                actor->last_x = actor->x;
                actor->y      = (rand() % 50) * 4;
                actor->last_y = actor->y;
                actor->dy     = -1;
                actor->axis   = 0;
                break;
            case 3:
                actor->x      = (rand() % 50) * 4;
                actor->last_y = actor->x;
                actor->y      = (rand() & 3) | 64;
                actor->last_x = actor->y;
                actor->dx     = -1;
                actor->axis   = 1;
                break;
        }
        if (TILE(actor->x, actor->y) & 0x800)
            i--;
        else if (func_80094F2C(actor, 1) && func_80094F2C(actor, -1))
            i--;
        else
        {
            if (direction / 2 == 0)
            {
                directional++;
                if (directional >= 96)
                    directional = 95;
            }
            TILE(actor->x, actor->y) |= 0x800;
            actor++;
        }
    }
}
