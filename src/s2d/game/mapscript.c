extern unsigned char *D_800B9188;
extern unsigned short D_800B91EC[];
extern int state __asm__("D_800AA73C+0x5C1C");
extern int rand(void), EventFlagTest(void *);
extern unsigned short S2dIndicatorState(void);
extern void S2dEventFlagSet(unsigned short),
    S2dWinInit(int, void *, void *, int, int, int, int, int, int, int, int), func_8009AC18(void *),
    func_80093AE0(void), S2dIndicatorOpen(void), S2dIndicatorClose(void), func_8009C67C(int),
    func_8009C308(int), func_8009C33C(void);
int func_8009C07C(void)
{
    unsigned char *p = D_800B9188;
    if (*p == 255)
    {
        p++;
        switch (*p)
        {
        case 0x24:
            p++;
            S2dEventFlagSet(*(unsigned short *)p);
            p += 2;
            break;
        case 0x20:
            p += 3;
            break;
        case 0x21:
            return 1;
        case 0x23:
            if ((unsigned char)(rand() % 256) <= p[1])
            {
            case 0x22:
                p = *(unsigned char **)(p + 3);
            }
            else
                p += 8;
            break;
        case 0x26:
            p++;
            if (EventFlagTest(p) == 1)
                p = *(unsigned char **)(p + 2);
            else
                p += 6;
            break;
        case 0x55:
            S2dWinInit(133, D_800B91EC, *(void **)(p + 3), 640, 256, 512, 248, 15, 3, -216, 72);
            do
            {
                func_8009AC18(D_800B91EC);
                func_80093AE0();
            } while (D_800B91EC[1]);
            p += 7;
            break;
        case 0x60:
            S2dIndicatorOpen();
            while (S2dIndicatorState())
                func_80093AE0();
            p += 3;
            break;
        case 0x61:
            S2dIndicatorClose();
            while (S2dIndicatorState())
                func_80093AE0();
            p += 3;
            break;
        case 0x67:
            p++;
            func_8009C67C(*p++);
            func_8009C308(*p);
            while (state != 2)
                func_80093AE0();
            p++;
            break;
        case 0x68:
            func_8009C33C();
            while (state)
                func_80093AE0();
            p += 3;
            break;
        }
    }
    else
        p++;
    D_800B9188 = p;
    return 0;
}
