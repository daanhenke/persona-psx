#include <persona/casino/reels.h>

extern ReelState D_800AF834;
extern CasinoLayout *D_800A142C,*D_800A1248;
extern unsigned int D_800A8EA4,g_casino_money_shown;
extern CasinoPalCycle cycles[] __asm__("D_80099D08+4");
extern CasinoPalCycle active_cycles[] __asm__("D_80099D20");
extern CasinoObj D_800A16F8,D_800A14DC,D_800A156C;
extern void CasinoShowNumber(unsigned int,CasinoLayout *,u_char);
extern void CasinoStartPalCycle(CasinoPalCycle *);
extern void CasinoStartAnim(CasinoObj *,short,short,short,short,long,long,long,long,long,long);
extern void func_80085CF8(void),func_80086C4C(int),func_80086B88(int),func_80086D40(int),func_80086EEC(int);
extern void func_800867A8(u_char),func_800868A0(u_char);
void func_8008634C(void)
{
 int i;
 if(g_casino_step==0x73||g_casino_step==0x71)func_80085CF8();
 if(g_casino_money!=g_casino_money_shown){CasinoShowNumber(g_casino_money,D_800A142C,8);g_casino_money_shown=g_casino_money;}
 if(g_casino_win!=D_800A8EA4){CasinoShowNumber(g_casino_win,D_800A1248,6);D_800A8EA4=g_casino_win;}
 if(g_casino_bet<9U&&g_casino_bet!=0&&active_cycles[g_casino_bet-1].on==0)CasinoStartPalCycle(&cycles[g_casino_bet]);
 if(CASINO_LAMPS.b4!=CASINO_LAMPS.b5){
  int angle=-2048;if(CASINO_LAMPS.b4)angle=2048;
  CasinoStartAnim(&D_800A16F8,32,angle,0,0,0,0,0,0,0,0);CASINO_LAMPS.b5=CASINO_LAMPS.b4;
 }
 if(CASINO_LAMPS.b0!=CASINO_LAMPS.b1){CasinoStartAnim(&D_800A14DC,16,2048,0,0,0,0,0,0,0,0);CASINO_LAMPS.b1=CASINO_LAMPS.b0;}
 if(CASINO_LAMPS.b2!=CASINO_LAMPS.b3){CasinoStartAnim(&D_800A156C,16,2048,0,0,0,0,0,0,0,0);CASINO_LAMPS.b3=CASINO_LAMPS.b2;}
 i=0;
 if(g_casino_step==0x70||g_casino_step==0x73||(unsigned)(g_casino_step-0x71)<2||g_casino_step==0x3C){
  D_800AF834.header[6]=0;
  for(i=0;i<21;i++){
   switch((signed char)D_800AF834.state[i]){case 0:break;
   case 1:if(D_800AF834.layouts[0][i]->busy==0){
    if(i<5)func_80086C4C(i);else func_80086B88(i);
    func_800867A8(i);D_800AF834.ticks[i]++;
   }break;
   case 2:if(D_800AF834.layouts[0][i]->busy==0)func_800867A8(i);break;
   case 3:if(D_800AF834.layouts[0][i]->busy==0){
    if(i<5)func_80086D40(i);else func_80086EEC(i);
    func_800867A8(i);D_800AF834.ticks[i]++;
   }break;
   }
   if(D_800AF834.layouts[0][i]->busy==1){
    D_800AF834.flags[i]+=D_800AF834.delta[i]/(signed char)D_800AF834.frames[i];func_800868A0(i);
   }
  }
 }
}
