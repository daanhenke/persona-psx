#include <persona/casino/reels.h>
#include <libsnd.h>

extern ReelState D_800AF834;
extern signed char D_800AF9E0[];
extern unsigned char *D_800B3E3C[];
extern CasinoLayout *D_800A24D8[][2];
extern void CasinoTween(CasinoLayout *,int,short,short,short,short);
void func_800871BC(unsigned char group)
{
 unsigned char reset[4],expected[4],symbols[4],previous[4];
 unsigned char *rows[4];
 unsigned char which;
 short dx,dy;
 int i;
 SsSeqStop(g_casino_seqs[10]);SsSeqPlay(g_casino_seqs[10],1,1);
 for(i=0;i<4;i++){
  which=(i+D_800AF9E0[group])%4;
  switch(which){
  case 0:dx=0;dy=-16;break;
  case 1:dx=16;dy=0;break;
  case 2:dx=0;dy=16;break;
  case 3:dx=-16;dy=0;break;
  }
  CasinoTween(D_800A24D8[i+group*4][0],dx,dy,0,0,16);
  reset[i]=D_800AF834.reset[group*4+which+5];
  expected[i]=D_800AF834.expected[group*4+which+5];
  symbols[i]=D_800AF834.symbols[group*4+which+5];
  previous[i]=D_800AF834.previous[group*4+which+5];
  rows[i]=D_800B3E3C[group*4+which];
 }
 for(i=0;i<4;i++){
  {int rotation=D_800AF9E0[group]-3;which=(i-rotation)%4;}
  D_800AF834.reset[group*4+i+5]=reset[which];
  D_800AF834.expected[group*4+i+5]=expected[which];
  D_800AF834.symbols[group*4+i+5]=symbols[which];
  D_800AF834.previous[group*4+i+5]=previous[which];
  D_800B3E3C[group*4+i]=rows[which];
  D_800AF834.layouts[1][group*4+i+5]->tex_idx[0]=D_800B3E3C[group*4+i][(signed char)D_800AF834.reset[group*4+i+5]];
 }
}
