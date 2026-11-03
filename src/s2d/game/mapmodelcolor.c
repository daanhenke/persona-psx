/* Persona 1 (JP): animate the map model palette and copy colors to primitives. */
typedef struct
{
    unsigned char r, g, b, code;
} Color;
typedef struct
{
    int tag;
    Color color[4];
    unsigned short index[4];
} Quad;
typedef struct
{
    char pad[28];
    Quad *quads;
} State;
extern int g_s2d_frame;
extern Color D_800B56F0[];
extern Color D_800B56F4;
extern int rsin(int), rcos(int);
void func_8009777C(State *state)
{
    Color *palette;
    Quad *quad;
    int i, j, mode;
    Color *color, *first;
    unsigned short *index;
    palette = D_800B56F0;
    quad = state->quads;
    palette[0].r = (((rsin((g_s2d_frame * 163) & 4095) + 4096) / 2) * 191) >> 12;
    D_800B56F0[0].g = 23;
    D_800B56F0[0].b = (((rsin((g_s2d_frame * 37) & 4095) + 4096) / 2) * 191) >> 12;
    D_800B56F4.r = (((rcos((g_s2d_frame * 273) & 4095) + 4096) / 2) * 191) >> 12;
    D_800B56F4.g = (((rcos((g_s2d_frame * 51) & 4095) + 4096) / 2) * 191) >> 12;
    D_800B56F4.b = 34;
    for (i = 0, mode = 57, first = quad->color; i < 7; i++)
    {
        j = 0;
        color = first;
        index = quad->index;
        for (; j < 4; j++)
        {
            *color++ = *(Color *)((int)palette + ((*index++ << 1) & 4));
        }
        first[0].code = mode;
        first = (Color *)((char *)first + 28);
        quad++;
    }
}
