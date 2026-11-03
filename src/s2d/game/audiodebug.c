/* Persona 1 (JP): audio debug menu sequence controls. */
extern int          D_800A4D0C, D_800A4D1C, D_800A4D20;
extern short        SsSeqOpen(unsigned long*, short);
extern void         SsSeqPause(short), SsSeqPlay(short, char, short), SsSeqReplay(short), SsSeqSetVol(short, short, short), SsSeqStop(short), SsSetNck(short);
extern int          func_8008BCC8(int, int, int);
extern unsigned int func_8008E158(int);
void                func_8008C038(void)
{
    short h;
    short opened;
    switch (D_800A4D1C)
    {
        case 0:
            if (func_8008E158(1) & 32)
            {
                D_800A4D0C ^= 1;
                if (D_800A4D0C)
                    SsSeqReplay(*(short*)0x801F537C);
                else
                    SsSeqPause(*(short*)0x801F537C);
            }
            break;
        case 1:
        case 2:
        case 3:
            (&D_800A4D0C)[D_800A4D1C] = func_8008BCC8((&D_800A4D0C)[D_800A4D1C], 0, 24);
            if (func_8008E158(1) & 32)
            {
                h = ((short*)0x801F538E)[D_800A4D1C];
                if (h != -1)
                {
                    SsSeqStop(h);
                    SsSetNck(((short*)0x801F538E)[D_800A4D1C]);
                    ((short*)0x801F538E)[D_800A4D1C] = -1;
                }
                else
                {
                    opened                           = SsSeqOpen((unsigned long*)(((int*)0x801D8004)[(&D_800A4D0C)[D_800A4D1C]] + 0x801D8000), *(short*)0x801F535E);
                    ((short*)0x801F538E)[D_800A4D1C] = opened;
                    SsSeqSetVol(opened, 126, 126);
                    SsSeqPlay(((short*)0x801F538E)[D_800A4D1C], 1, 1);
                }
            }
            break;
        case 4:
            if (func_8008E158(1) & 32)
            {
                D_800A4D1C = 0;
                D_800A4D20++;
            }
            break;
    }
    if (func_8008E158(2) & 4096)
    {
        D_800A4D1C--;
        if (D_800A4D1C < 0)
            D_800A4D1C = 4;
    }
    if (func_8008E158(2) & 16384)
    {
        D_800A4D1C++;
        if (D_800A4D1C >= 5)
            D_800A4D1C = 0;
    }
    if (func_8008E158(1) & 256)
    {
        D_800A4D1C = 0;
        D_800A4D20++;
    }
    if (func_8008E158(1) & 64)
    {
        D_800A4D1C = 0;
        D_800A4D20++;
    }
}
