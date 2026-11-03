#include <persona/s2d/s2d.h>

extern u_long g_model_objs[256];

/* Keep the full-width index in the accessor after the caller narrows map. */
static __inline__ S2dMapInfo* MapInfoAt(S2dMapInfo* base, int index)
{
    return base + index;
}

void func_80097BE8(u_long* tmd, short map)
{
    u_long*     object;
    u_long*     primitive_count;
    u_short*    ids;
    S2dMapInfo* info;
    u_long*     models;
    GsMapModelingData(tmd += 1);
    object  = tmd;
    info    = MapInfoAt(g_map_info, map);
    object += 2;
    if (info->pad[0] != 255)
    {
        models          = g_model_objs;
        ids             = info->pad;
        primitive_count = object + 5;
    again:
        {
            models[*ids]                   = (u_long)object;
            models[*ids]                  &= 0x7FFFFFFF;
            object                        += 7;
            *(u_long*)primitive_count[-1] &= 0xFF000000;
            *(u_long*)primitive_count[-1] |= *primitive_count;
            ids++;
            primitive_count += 7;
        }
        if (*ids != 255)
            goto again;
    }
}
