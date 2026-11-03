/* Open or close the map's two visible fixed-point scale components. */
#include <decomp/types.h>
typedef struct { long x, y, z; } MapScale;
extern MapScale map_scale __asm__("D_800B5F2C");
extern long scale_state __asm__("D_800AA73C+0x5C1C");

void func_8009C36C(void)
{
    switch (scale_state) {
    case 1: {
        MapScale *const scale = &map_scale;
        scale->x += 0x180;
        if (scale->x > 0x1000) scale->x = 0x1000;
        scale->y += 0x180;
        if (scale->y > 0x1000) scale->y = 0x1000;
        if (scale->x >= 0x1000 && scale->y >= 0x1000) scale_state = 2;
        break;
    }
    case 3: {
        MapScale *const scale = &map_scale;
        scale->x -= 0x180;
        if (scale->x < 0) scale->x = 0;
        scale->y -= 0x180;
        if (scale->y < 0) scale->y = 0;
        if (scale->x <= 0 && scale->y <= 0) scale_state = 0;
        break;
    }
    }
}
