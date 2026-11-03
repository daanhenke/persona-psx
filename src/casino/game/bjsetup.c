/* Persona 1 (JP): construct the blackjack table graphics. */
typedef struct
{
    int d[5];
} Obj;
typedef struct
{
    int   d[3];
    void* xform;
} Draw;
extern unsigned char  D_8009A378[], D_8009A438[], D_8009A490[], D_8009A4D0[], D_8009A518[], D_8009A690[], D_8009A718[], D_8009A758[], D_8009A7A0[], D_8009A8EC[], D_8009A96C[], D_8009A9AC[], D_8009A9F4[], D_8009AA64[], D_8009AABC[], D_8009AAFC[], D_8009AB44[], D_8009AC0C[], D_8009ACE4[], D_8009AD88[], D_8009AE2C[], D_8009AED0[], D_8009B2B4[], D_8009B30C[], D_8009B394[], D_8009B3E4[], D_8009B484[], D_8009B4DC[], D_8009B570[], D_8009B5C0[], D_8009B600[], D_8009B648[], D_8009B688[], D_8009B6D8[], D_8009B7C0[], D_8009B81C[], D_8009B874[], D_8009B91C[], D_8009B97C[], D_8009B9E4[], D_8009BA64[], D_8009BAF4[], D_8009BB9C[], D_8009BBAC[], D_8009BC14[], D_8009BC24[], D_8009BC8C[], D_8009BC9C[], D_8009BD04[], D_8009BD14[], D_8009BD98[], D_8009BDA8[], D_8009BE58[], D_8009BE68[], D_8009BF28[], D_8009BF38[], D_8009BFE8[], D_8009BFF8[], D_8009C078[], D_8009C088[], D_8009C108[], D_8009C118[], g_bj_palanim0[];
extern int            D_8009BBA8, D_8009BC20, D_8009BC98, D_8009BD10, D_8009BDA4, D_8009BE64, D_8009BF34, D_8009BFF4, D_8009C084, D_8009C114;
extern void           CasinoBuildFrame(void*), CasinoBuildLayouts(void*, int), CasinoBuildObj(void*), CasinoDrawObj(void*), CasinoQueueCluts(short, short, int), CasinoQueueSemi(short, short, unsigned char), CasinoSetXform(void*), CasinoTexObj(void*);
extern unsigned short GetClut(int, int);
extern Obj            D_8009C2D4[];
extern Draw           D_8009C274[];
extern Obj            D_8009C44C[];
extern Draw           D_8009C41C[];
extern Obj            D_8009C588[];
extern Draw           D_8009C558[];
extern int            D_8009B7B0[];
extern void*          blackjack_xform __asm__("D_8009B7B0+12");
void                  func_8007DBC8(void)
{
    short i;
    CasinoBuildLayouts(D_8009A378, 0);
    CasinoBuildLayouts(D_8009A438, 0);
    CasinoBuildLayouts(D_8009A490, 0);
    CasinoBuildFrame(D_8009A4D0);
    CasinoBuildLayouts(D_8009A518, 0);
    CasinoBuildLayouts(D_8009A690, 0);
    CasinoBuildLayouts(D_8009A718, 0);
    CasinoBuildFrame(D_8009A758);
    CasinoBuildLayouts(D_8009A7A0, 0);
    CasinoBuildLayouts(D_8009A8EC, 0);
    CasinoBuildLayouts(D_8009A96C, 0);
    CasinoBuildFrame(D_8009A9AC);
    CasinoBuildLayouts(D_8009A9F4, 0);
    CasinoBuildLayouts(D_8009AA64, 0);
    CasinoBuildLayouts(D_8009AABC, 0);
    CasinoBuildFrame(D_8009AAFC);
    CasinoBuildLayouts(D_8009AB44, 0);
    CasinoBuildLayouts(D_8009AC0C, 0);
    CasinoBuildLayouts(D_8009ACE4, 0);
    CasinoBuildLayouts(D_8009AD88, 0);
    CasinoBuildLayouts(D_8009AE2C, 0);
    CasinoBuildLayouts(D_8009AED0, 0);
    CasinoBuildLayouts(D_8009B2B4, 0);
    CasinoBuildLayouts(D_8009B2B4 + 8, 0);
    CasinoBuildLayouts(D_8009B2B4 + 0x10, 0);
    CasinoBuildLayouts(D_8009B2B4 + 0x18, 0);
    CasinoBuildFrame(D_8009B30C);
    CasinoBuildLayouts(D_8009B394, 0);
    CasinoBuildLayouts(D_8009B3E4, 0);
    CasinoBuildLayouts(D_8009B484, 0);
    CasinoBuildLayouts(D_8009B4DC, 0);
    CasinoBuildLayouts(D_8009B570, 0);
    CasinoBuildFrame(D_8009B600);
    CasinoBuildLayouts(D_8009B5C0, 0);
    CasinoBuildFrame(D_8009B688);
    CasinoBuildLayouts(D_8009B648, 0);
    CasinoBuildLayouts(D_8009B91C, 0);
    CasinoBuildLayouts(D_8009B97C, 0);
    CasinoQueueSemi(0x39B, 1, 1);
    CasinoBuildLayouts(D_8009B9E4, 0);
    CasinoBuildLayouts(D_8009BA64, 0);
    CasinoBuildLayouts(D_8009BAF4, 0);
    CasinoBuildLayouts(D_8009B6D8, 0);
    CasinoBuildLayouts(D_8009B81C, 0);
    CasinoBuildLayouts(D_8009B874, 0);
    CasinoBuildObj(D_8009B7C0);
    CasinoTexObj(D_8009B7C0);
    CasinoSetXform(blackjack_xform);
    CasinoDrawObj(&D_8009B7B0[0]);
    CasinoBuildObj(D_8009BBAC);
    CasinoTexObj(D_8009BBAC);
    CasinoSetXform((void*)D_8009BBA8);
    CasinoDrawObj(D_8009BB9C);
    CasinoBuildObj(D_8009BC24);
    CasinoTexObj(D_8009BC24);
    CasinoSetXform((void*)D_8009BC20);
    CasinoDrawObj(D_8009BC14);
    CasinoBuildObj(D_8009BC9C);
    CasinoTexObj(D_8009BC9C);
    CasinoSetXform((void*)D_8009BC98);
    CasinoDrawObj(D_8009BC8C);
    CasinoBuildObj(D_8009BD14);
    CasinoTexObj(D_8009BD14);
    CasinoSetXform((void*)D_8009BD10);
    CasinoDrawObj(D_8009BD04);
    CasinoBuildObj(D_8009BDA8);
    CasinoTexObj(D_8009BDA8);
    CasinoSetXform((void*)D_8009BDA4);
    CasinoDrawObj(D_8009BD98);
    CasinoBuildObj(D_8009BE68);
    CasinoTexObj(D_8009BE68);
    CasinoSetXform((void*)D_8009BE64);
    CasinoDrawObj(D_8009BE58);
    CasinoBuildObj(D_8009BF38);
    CasinoTexObj(D_8009BF38);
    CasinoSetXform((void*)D_8009BF34);
    CasinoDrawObj(D_8009BF28);
    CasinoBuildObj(D_8009BFF8);
    CasinoTexObj(D_8009BFF8);
    CasinoSetXform((void*)D_8009BFF4);
    CasinoDrawObj(D_8009BFE8);
    CasinoBuildObj(D_8009C088);
    CasinoTexObj(D_8009C088);
    CasinoSetXform((void*)D_8009C084);
    CasinoDrawObj(D_8009C078);
    CasinoBuildObj(D_8009C118);
    CasinoTexObj(D_8009C118);
    CasinoSetXform((void*)D_8009C114);
    CasinoDrawObj(D_8009C108);
    for (i = 0; i < 6; i++)
    {
        CasinoBuildObj(&D_8009C2D4[i]);
        CasinoTexObj(&D_8009C2D4[i]);
        CasinoSetXform(D_8009C274[i].xform);
        CasinoDrawObj(&D_8009C274[i]);
    }
    CasinoQueueCluts(875, 24, 31844);
    for (i = 0; i < 3; i++)
    {
        CasinoBuildObj(&D_8009C44C[i]);
        CasinoTexObj(&D_8009C44C[i]);
        CasinoSetXform(D_8009C41C[i].xform);
        CasinoDrawObj(&D_8009C41C[i]);
    }
    CasinoQueueCluts(899, 12, 31908);
    for (i = 0; i < 3; i++)
    {
        CasinoBuildObj(&D_8009C588[i]);
        CasinoTexObj(&D_8009C588[i]);
        CasinoSetXform(D_8009C558[i].xform);
        CasinoDrawObj(&D_8009C558[i]);
    }
    CasinoQueueCluts(911, 12, 31972);
    CasinoQueueCluts(0x338, 14, GetClut(*(short*)(g_bj_palanim0 + 8), *(short*)(g_bj_palanim0 + 10)));
    CasinoQueueCluts(0x346, 7, GetClut(*(short*)(g_bj_palanim0 + 8), *(short*)(g_bj_palanim0 + 10)));
}
