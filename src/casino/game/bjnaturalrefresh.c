extern int g_casino_timer;
extern unsigned char D_800B09F4[],g_casino_bet,D_800A8768[],g_bj_play_step,g_bj_natural[];
extern short D_80099990[];
extern unsigned short GetClut(short,short);
extern void func_8007C514(int,int),func_8007C8E4(int,int),func_8007CA5C(int,int),func_8007F7E8(int),CasinoQueueCluts(int,int,unsigned short);

void func_8007C358(void)
{
    if (g_casino_timer == 0)
        g_casino_bet = D_800B09F4[1];
    switch(D_800A8768[1])
    {
    case 11:
        func_8007C514(1,48);
        break;
    case 12:
        func_8007C8E4(1,48);
        break;
    case 13:
        func_8007CA5C(1,48);
        break;
    }
    if(g_bj_play_step!=121)
    {
        func_8007F7E8(1);
        if(D_800A8768[0]==11)
        {
            CasinoQueueCluts(g_bj_natural[0]+748,1,GetClut(D_80099990[4],D_80099990[5]));
            CasinoQueueCluts(g_bj_natural[0]+755,1,GetClut(D_80099990[4],D_80099990[5]));
        }
        if(D_800A8768[1]==11)
        {
            CasinoQueueCluts(g_bj_natural[1]+748,1,GetClut(D_80099990[4],D_80099990[5]));
            CasinoQueueCluts(g_bj_natural[1]+755,1,GetClut(D_80099990[4],D_80099990[5]));
        }
    }
}
