/* Persona 1 (JP): timed grid prompt blink and completion. */
typedef struct
{
    int    pad[5];
    short* state;
} Layout;
extern Layout*     grid_prompt __asm__("D_800A2BC8+868");
extern short       grid_timer __asm__("D_800AF9E8+208");
extern signed char D_800A8704;
extern void        CasinoShowLayout(void*, unsigned char), CasinoTween(void*, int, short, short, short, short), CasinoSpritesSetOn(short, short, int);
void               func_8008C2DC(short duration)
{
    switch (grid_timer)
    {
        case 0:
            *grid_prompt->state = 4;
            CasinoShowLayout(grid_prompt, 0);
            CasinoTween(grid_prompt, -128, 0, 0, 0, 1);
            break;
        case 16:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 32:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 48:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 64:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 80:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 96:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 112:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 128:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 144:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 160:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 176:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 192:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 208:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 224:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 240:
            CasinoSpritesSetOn(16, 1, 0);
            break;
        case 24:
        case 40:
        case 56:
        case 72:
        case 88:
        case 104:
        case 120:
        case 136:
        case 152:
        case 168:
        case 184:
        case 200:
        case 216:
        case 232:
        case 248:
            CasinoSpritesSetOn(16, 1, 1);
            break;
    }
    if (duration == grid_timer)
    {
        CasinoShowLayout(grid_prompt, 1);
        D_800A8704 = 1;
    }
    grid_timer++;
}
