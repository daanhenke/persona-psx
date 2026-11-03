typedef struct
{
    int header;
    short pages[16];
} DrawModePages;

extern DrawModePages D_800B56F4;

void func_80097DBC(int index, short page)
{
    D_800B56F4.pages[index] = page;
}

void func_80097DD4(int index)
{
    D_800B56F4.pages[index] = -1;
}
