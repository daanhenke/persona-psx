/* Count exact-position and displaced matches in each grid row and column. */
extern signed char D_800A87E0[3][3], D_800AFC00[3][3];
extern signed char D_800B4758[4][3];

int func_8008C9DC(void)
{
    int row, col, other;

    D_800B4758[0][0] = D_800B4758[0][1] = D_800B4758[0][2] =
        D_800B4758[1][0] = D_800B4758[1][1] = D_800B4758[1][2] =
        D_800B4758[2][0] = D_800B4758[2][1] = D_800B4758[2][2] =
        D_800B4758[3][0] = D_800B4758[3][1] = D_800B4758[3][2] = 0;

    for (row = 0; row < 3; row++)
    {
        for (col = 0; col < 3; col++)
        {
            for (other = 0; other < 3; other++)
            {
                if (D_800AFC00[row][col] == D_800A87E0[row][other])
                {
                    if (col == other)
                        D_800B4758[0][row]++;
                    else
                        D_800B4758[1][row]++;
                }
            }
        }
    }
    for (col = 0; col < 3; col++)
    {
        for (row = 0; row < 3; row++)
        {
            for (other = 0; other < 3; other++)
            {
                if (D_800AFC00[row][col] == D_800A87E0[other][col])
                {
                    if (row == other)
                        D_800B4758[2][col]++;
                    else
                        D_800B4758[3][col]++;
                }
            }
        }
    }
    return D_800B4758[0][0] + D_800B4758[0][1] + D_800B4758[0][2] +
               D_800B4758[2][0] + D_800B4758[2][1] + D_800B4758[2][2] == 18;
}
