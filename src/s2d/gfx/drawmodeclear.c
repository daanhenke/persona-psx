typedef struct
{
    int header;
    short pages[16];
} DrawModePages;

extern DrawModePages D_800B56F4;

void D_80097CD4(void)
{
    int i;
    int empty = -1;

    for (i = 15; i >= 0; i--)
        D_800B56F4.pages[i] = empty;
}
