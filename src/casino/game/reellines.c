extern unsigned char D_800A85BC[][5], D_800A85F8[];
extern void func_80087818(int, int, int, int);

void func_800876FC(signed char input[][5], unsigned char count)
{
    int row, symbol, hits, column, width;
    int rows = count;

    if ((unsigned int)rows >= 5)
        rows = count + (count - 4);
    for (row = 0; row < rows; row++)
    {
        D_800A85F8[row] = 0;
        for (symbol = 0; symbol < 7; symbol++)
        {
            width = row < 4 ? 5 : 4;
            hits = 0;
            for (column = 0; column < width; column++)
            {
                if (input[row][column] == symbol)
                {
                    hits++;
                    D_800A85BC[row][column] = 1;
                }
                else
                    D_800A85BC[row][column] = 0;
            }
            if (hits >= 3)
            {
                func_80087818(symbol, hits, width, row);
                break;
            }
        }
    }
}
