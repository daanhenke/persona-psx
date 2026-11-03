/* Slide reward objects away while applying the eleven payout entries. */
extern int g_casino_timer;
extern unsigned int g_casino_win,D_800A96A4[];
extern unsigned char g_casino_step,D_800A2BC8[],D_800B0720[];
extern int D_800B0724[];extern int reward_right_x __asm__("D_800B0814+0x8");
extern void func_80091154(unsigned char,unsigned char),func_80090EEC(unsigned char),func_8009019C(unsigned int *),func_80092430(void *),func_800925DC(void),func_80092910(void);
void func_8008E590(void){
 int moving=0;
 if(g_casino_timer>=0){
  if(D_800B0724[2]>=-191){D_800B0724[2]-=16;moving=1;}
  if(reward_right_x>=-191){reward_right_x-=16;moving++;}
 }
 if((unsigned int)g_casino_timer<22 && !(g_casino_timer&1)){
  func_80091154(g_casino_timer/2,D_800A2BC8[16+g_casino_timer/2]);
  D_800A96A4[g_casino_timer/2]=g_casino_win;
  func_80090EEC(g_casino_timer/2);
  func_8009019C(D_800A96A4);
 }
 if(moving==0 && g_casino_timer>=22){
  func_80092430(D_800B0720);func_800925DC();
  func_80092430(D_800B0720+240);func_80092910();
  g_casino_step=112;g_casino_timer=-1;
 }
}
