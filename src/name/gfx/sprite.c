/* Persona 1 (JP) - the name screen's sprites.  NAME @ 0x80067BE0.
 *
 * Fifty GsSPRITEs, each with an attribute word of its own beside it: the low
 * nibble is the OT depth, 0x40 picks the full GsSortSprite over the fast one,
 * 0x80 hides the sprite, and 0x20 marks it semi-transparent from the start.
 */
#include <decomp/types.h>
#include <persona/name/entry.h>

void NameSpriteSet(u_short i, u_short w, u_short h, u_short tpage, u_short u,
                   u_short v, short cx, short cy, int attr)
{
    g_sprite_attr[i] = attr;
    g_sprites[i].attribute = !(attr & 0x20) ? 0x1000000 : 0;
    g_sprites[i].mx = w >> 1;
    g_sprites[i].my = h >> 1;
    g_sprites[i].r = 0x80;
    g_sprites[i].g = 0x80;
    g_sprites[i].b = 0x80;
    g_sprites[i].w = w;
    g_sprites[i].h = h;
    g_sprites[i].tpage = tpage;
    g_sprites[i].u = u;
    g_sprites[i].v = v;
    g_sprites[i].cx = cx;
    g_sprites[i].cy = cy;
    g_sprites[i].rotate = 0;
    g_sprites[i].scalex = 0x1000;
    g_sprites[i].scaley = 0x1000;
}

/* Sets sprite `i` up from its entry in the table. */
void NameSpriteFromDef(u_char i)
{
    NameSpriteSet(i, g_name_sprite_defs[i].w, g_name_sprite_defs[i].h,
                  g_name_sprite_defs[i].tpage, g_name_sprite_defs[i].u,
                  g_name_sprite_defs[i].v, g_name_sprite_defs[i].cx,
                  g_name_sprite_defs[i].cy, g_name_sprite_defs[i].attr);
    g_sprites[i].x = g_name_sprite_defs[i].x;
    g_sprites[i].y = g_name_sprite_defs[i].y;
}
