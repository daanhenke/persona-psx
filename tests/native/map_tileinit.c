/* Native test: complete tile arrays, sentinels, duplicate coordinates and callbacks. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <decomp/types.h>
static union { u_int align; u_char bytes[160256]; } map_storage;
static u_char expected[160256], packed_storage[20032];
u_char D_800A5168[34][2], D_800A519C[34][2];
u_char D_800A51F0[34][2], D_800A5314[34][2];
u_char D_800B0FAC[3488];
u_short g_map_id;
u_char *native_packed_map;
/* The production pointer's symbol is four bytes into a PS1 record. Give that
 * pointer a naturally aligned host object; no production address is changed. */
__asm__(".set D_800B1D30,native_packed_map-4");
#define PERSONA_TILE_MAP map_storage.bytes
#include "../../src/s2d/game/tileinit.c"
static unsigned old_offset, new_offset;
static int calls, mutate, error;
static unsigned random_state;
static unsigned next_random(void)
{
    random_state = random_state * 1664525u + 1013904223u;
    return random_state;
}
static void fail(const char *what)
{
    fprintf(stderr, "tile initialization: %s\n", what); exit(1);
}
void *bzero(u_char *p, int n)
{
    int i;
    if (calls == 0)
    {
        if (p != packed_storage + 16 || n != 20000) fail("packed clear arguments");
        if (memcmp(map_storage.bytes, expected, sizeof expected)) fail("tile clear before callback");
        for (i = 0; i < sizeof D_800B0FAC; i++)
            if (D_800B0FAC[i] != 0x5A) fail("secondary buffer cleared early");
    }
    else if (calls == 1)
    {
        if (p != D_800B0FAC || n != 3456) fail("secondary clear arguments");
        for (i = 16; i < 20016; i++)
            if (packed_storage[i]) fail("packed buffer not cleared");
        if (mutate)
        {
            /* Calls can replace the map header and determine the later map test. */
            *(u_int *)map_storage.bytes = new_offset;
            *(u_int *)expected = new_offset;
            g_map_id = (mutate == 1) ? 0 : 0x8000;
            D_800A51F0[0][0] = 199;
            D_800A51F0[0][1] = 199;
        }
    }
    else fail("extra buffer clear");
    memset(p, 0, (size_t)n);
    calls++;
    return p;
}
void func_80094660(void)
{
    u_char (*lists[4])[2] = {D_800A51F0,D_800A5314,D_800A5168,D_800A519C};
    unsigned bits[4] = {0x3000,0x5000,0x2000,0x4000};
    int a, i;
    if (calls != 2) fail("final callback order");
    if (g_map_id == 0)
        for (a = 0; a < 4; a++)
            for (i = 0; lists[a][i][0] != 255; i++)
            {
                unsigned index = lists[a][i][1] * 200u + lists[a][i][0];
                ((u_short *)(expected + *(u_int *)expected))[index] |= bits[a];
            }
    if (memcmp(expected, map_storage.bytes, sizeof expected)) fail("complete tile result");
    calls++;
}
int main(void)
{
    const u_short ids[] = {0,1,0x7FFF,0x8000,0xFFFF};
    u_char (*lists[4])[2] = {D_800A51F0,D_800A5314,D_800A5168,D_800A519C};
    unsigned seed, mode, id, a, i, fixtures = 0;
    for (seed = 0; seed < 64; seed++)
    for (mode = 0; mode < 3; mode++)
    for (id = 0; id < sizeof ids / sizeof ids[0]; id++)
    {
        random_state = seed + 19;
        for (i = 0; i < sizeof map_storage.bytes; i++)
            map_storage.bytes[i] = next_random() >> 24;
        old_offset = (seed & 1) ? 16 : 128;
        new_offset = 80128;
        *(u_int *)map_storage.bytes = old_offset;
        memcpy(expected, map_storage.bytes, sizeof expected);
        for (i = 0; i < 40000; i++)
            ((u_short *)(expected + old_offset))[i] &= 0x87FF;
        memset(packed_storage, 0xA5, sizeof packed_storage);
        memset(D_800B0FAC, 0x5A, sizeof D_800B0FAC);
        for (a = 0; a < 4; a++)
        {
            unsigned length = (seed + a * 7) % 33;
            memset(lists[a], 0xEE, 68);
            for (i = 0; i < length; i++)
            {
                lists[a][i][0] = (i & 3) ? next_random() % 200 : 199;
                lists[a][i][1] = (i & 3) ? next_random() % 200 : 199;
            }
            lists[a][length][0] = 255;
            /* Keep a second sentinel if the callback replaces an empty list. */
            lists[a][length + 1][0] = 255;
        }
        native_packed_map = packed_storage + 16;
        g_map_id = ids[id]; calls = 0; mutate = mode;
        func_80094B08();
        if (calls != 3) fail("missing callback");
        for (i = 0; i < sizeof packed_storage; i++)
            if (packed_storage[i] != ((i >= 16 && i < 20016) ? 0 : 0xA5)) fail("packed canary");
        for (i = 0; i < sizeof D_800B0FAC; i++)
            if (D_800B0FAC[i] != (i < 3456 ? 0 : 0x5A)) fail("secondary canary");
        fixtures++;
    }
    printf("tile initialization: %u complete-map fixtures passed\n", fixtures);
    return error;
}
