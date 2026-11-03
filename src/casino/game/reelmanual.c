extern signed char D_800AF738[], D_800AF73C[], D_800AF9E0[];
extern short *D_800A24D8, *D_800A24F8, *D_800A2518, *D_800A2538;
extern void func_800871BC(int);
void func_80085CF8(void)
{
    if (D_800AF73C[0] && !D_800A24D8[0])
    {
        D_800AF9E0[0]++;
        D_800AF9E0[0] %= 4;
        func_800871BC(0);
        D_800AF73C[0] = 0;
    }
    if (D_800AF738[3] && !D_800A24F8[0])
    {
        D_800AF9E0[1]++;
        D_800AF9E0[1] %= 4;
        func_800871BC(1);
        D_800AF738[3] = 0;
    }
    if (D_800AF738[2] && !D_800A2518[0])
    {
        D_800AF9E0[2]++;
        D_800AF9E0[2] %= 4;
        func_800871BC(2);
        D_800AF738[2] = 0;
    }
    if (D_800AF73C[1] && !D_800A2538[0])
    {
        D_800AF9E0[3]++;
        D_800AF9E0[3] %= 4;
        func_800871BC(3);
        D_800AF73C[1] = 0;
    }
}
