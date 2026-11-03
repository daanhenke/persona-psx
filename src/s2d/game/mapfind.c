/* Persona 1 (JP) - find the first of 96 records with matching byte coordinates. */
#include <decomp/types.h>

typedef struct {
    u_char x;
    u_char y;
    u_char rest[34];
} MapLookupRecord;

extern MapLookupRecord D_800B0FB0[];

int func_80094F9C(u_short x, u_short y)
{
    int i;

    for (i = 0; i < 96; i++) {
        if (D_800B0FB0[i].x == x && D_800B0FB0[i].y == y) {
            return i;
        }
    }
    return -1;
}
