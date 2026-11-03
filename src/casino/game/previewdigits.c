#include <persona/casino/casino.h>
typedef struct
{
    unsigned char current[10][2];
    unsigned char old[10][2];
} PreviewValues;
extern CasinoLayout *layouts[2][2] __asm__("D_800A2BC8+0x3148");
extern PreviewValues preview_values __asm__("D_800A9710-20");
/* The original caller passes promoted integer arguments. These sprite IDs
 * are 373..409, and the count is one, within the callee's short range. */
extern void PreviewSpritesSetOn(int, int, int) __asm__("CasinoSpritesSetOn");
extern void CasinoTexLayout(CasinoLayout *);
void func_80093614(void)
{
    int i;
    for (i = 0; i < 10; i++)
    {
        signed char total = preview_values.current[i][0] + preview_values.current[i][1];
        signed char previous = preview_values.old[i][0] + preview_values.old[i][1];
        if ((signed char)preview_values.current[i][1] != (signed char)preview_values.old[i][1])
        {
            layouts[i / 5][0]->tex_idx[(i * 4) % 20] = (signed char)preview_values.current[i][1] - 1;
            preview_values.old[i][1] = preview_values.current[i][1];
        }
        if ((signed char)preview_values.current[i][0] != (signed char)preview_values.old[i][0])
        {
            layouts[i / 5][0]->tex_idx[(i * 4) % 20 + 1] = (signed char)preview_values.current[i][0] - 1;
            preview_values.old[i][0] = preview_values.current[i][0];
        }
        if (total != previous)
        {
            if (total < 10)
            {
                layouts[i / 5][0]->tex_idx[(i * 4) % 20 + 2] = total + 6;
                PreviewSpritesSetOn(0x175 + i * 4, 1, 0);
            }
            else
            {
                layouts[i / 5][0]->tex_idx[(i * 4) % 20 + 2] = (signed char)(total / 10) + 6;
                layouts[i / 5][0]->tex_idx[(i * 4) % 20 + 3] = (signed char)(total % 10) + 6;
                PreviewSpritesSetOn(0x175 + i * 4, 1, 1);
            }
        }
    }
    CasinoTexLayout(layouts[0][0]);
    CasinoTexLayout(layouts[1][0]);
}
