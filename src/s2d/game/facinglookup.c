/* Translate the current facing through the original four-entry table. */
extern short g_s2d_facing;

int func_80094594(void)
{
    short directions[4] = {2, 1, 3, 0};
    return directions[g_s2d_facing];
}
