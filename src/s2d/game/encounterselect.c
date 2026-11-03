/* Select a pending encounter, or roll once the scene's delay has elapsed. */
#include <persona/s2d/s2d.h>
typedef struct { short value[4]; } EncounterChances;
extern EncounterChances D_80065050;
extern int D_800B9528, D_800B5F3C, g_key_lock;
extern int encounter_active __asm__("D_800B94EC+8");
extern u_short *encounter __asm__("D_800AA73C+0x5C28");
extern u_char D_800A4CE4;
extern short g_map_id;
extern int rand(void);
extern void *func_80096198(void);
#ifndef PERSONA_ENCOUNTER_FLAG
#define PERSONA_ENCOUNTER_FLAG (*(u_char *)0x801F5354)
#endif

int func_80096224(void)
{
    EncounterChances chances = D_80065050;
    if (g_fade.unk08 == 1) {
        if (encounter_active == 1) {
            int n = rand();
            D_800B5F3C = 0;
            {
                u_short map = encounter[n % 30];
                g_s2d_exit = 2;
                PERSONA_ENCOUNTER_FLAG = D_800A4CE4;
                g_map_id = map;
                goto done;
            }
        }
        D_800B9528++;
        if (D_800B9528 > 16) D_800B9528 = 16;
        if (D_800B9528 == 16) {
            encounter = func_80096198();
            if (rand() % 32 < chances.value[encounter[30]]) {
                int n;
                encounter_active = 1;
                n = rand();
                D_800B5F3C = 0;
                D_800B9528 = 0;
                g_key_lock = 0;
                g_s2d_exit = 2;
                PERSONA_ENCOUNTER_FLAG = D_800A4CE4;
                g_map_id = encounter[n % 30];
                return 1;
            }
        }
    }
done:
    return 0;
}
