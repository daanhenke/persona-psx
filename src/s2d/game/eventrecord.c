/* Persona 1 (JP) - choose the first scene record whose condition is unmet. */
#include <decomp/types.h>

extern u_char *scene_records[] __asm__("D_800A5314+0x330");
extern u_char g_btl_map_id;
extern int EventFlagTest(u_short *);

void *func_80096198(void)
{
    u_char *record = scene_records[g_btl_map_id];
    while (*(u_short *)(record + 62) != 0xFFFF) {
        if (EventFlagTest((u_short *)(record + 62)) == 0) {
            break;
        }
        record += 64;
    }
    return record;
}
