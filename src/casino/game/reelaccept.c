extern unsigned char D_800A85BC[][5], D_800A85C0[], D_800A85F8[];

void func_80087818(int symbol, int hits, int width, int row)
{
    int mask;

    if (hits == width)
        goto accept;
    if (hits == width - 1 &&
        (D_800A85BC[row][0] == 0 || D_800A85BC[row][width - 1] == 0))
        goto accept;
    if (width == 4)
        return;
    mask = ((D_800A85BC[row][0] & 1) << 4) |
           ((D_800A85BC[row][1] & 1) << 3) |
           ((D_800A85BC[row][2] & 1) << 2) |
           ((D_800A85BC[row][3] & 1) << 1) |
           (D_800A85C0[row * 5] & 1);
    switch (mask)
    {
    case 23:
        D_800A85BC[row][0] = 0;
        hits = 3;
        break;
    case 29:
        D_800A85C0[row * 5] = 0;
        hits = 3;
        break;
    case 7:
    case 14:
    case 28:
        hits = 3;
        break;
    default:
        return;
    }
accept:
    D_800A85F8[row] = hits;
    D_800A85F8[row + 13] = symbol;
}
