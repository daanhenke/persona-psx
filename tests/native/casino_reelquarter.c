#include "../../src/casino/game/reelquarter.c"
extern int printf(const char *,...);
extern void *memset(void *,int,unsigned long);
extern void *memcpy(void *,const void *,unsigned long);
extern int memcmp(const void *,const void *,unsigned long);
ReelState D_800AF834;
signed char D_800AF9E0[4];
unsigned char *D_800B3E3C[16];
CasinoLayout *D_800A24D8[16][2];
short g_casino_seqs[16];
static ReelState want;
static signed char rotations[4];
static unsigned char *want_rows[16],data[21][256];
static CasinoLayout layouts[42],*want_tweens[16][2];
static short textures[42][3],want_tex[42][3],sequence;
static int group,mode,step,bad,call_count,want_seq[2];
static struct {CasinoLayout *layout;int dx,dy;} calls[4];
static void mutate(ReelState *s,signed char *r,short *seq,unsigned char **rows,CasinoLayout *t[][2],int event)
{
 int k=group*4+(event%4),i=k+5;
 if((mode&1)&&event<2){*seq=(short)(*seq+7);r[group]=(r[group]+1)%4;}
 if((mode&2)&&event>=2){
  s->reset[i]^=0x85;s->expected[i]+=13;s->symbols[i]^=0xA7;s->previous[i]=(signed char)((unsigned char)s->previous[i]+31);
  r[group]=(r[group]+1)%4;
 }
 if((mode&4)&&event>=2){
  rows[k]=data[(k+event)%21]+128;s->layouts[1][i]=&layouts[21+i];
  t[(k+1)%16][0]=&layouts[(i+event)%42];
 }
}
void SsSeqStop(short n)
{if(step!=0||n!=want_seq[0])bad=1;mutate(&D_800AF834,D_800AF9E0,&g_casino_seqs[10],D_800B3E3C,D_800A24D8,step++);}
void SsSeqPlay(short n,char play,short repeat)
{if(step!=1||n!=want_seq[1]||play!=1||repeat!=1)bad=1;mutate(&D_800AF834,D_800AF9E0,&g_casino_seqs[10],D_800B3E3C,D_800A24D8,step++);}
void CasinoTween(CasinoLayout *l,int dx,short dy,short dw,short dh,short frames)
{
 int n=call_count++;
 if(step<2||step>5||n>3||l!=calls[n].layout||dx!=calls[n].dx||dy!=calls[n].dy||dw||dh||frames!=16)bad=1;
 mutate(&D_800AF834,D_800AF9E0,&g_casino_seqs[10],D_800B3E3C,D_800A24D8,step++);
}
static void reference(void)
{
 unsigned char saved[4][4];unsigned char *pointers[4];
 static const int dx[4]={0,16,0,-16},dy[4]={-16,0,16,0};
 int i,k,index;
 want_seq[0]=sequence;mutate(&want,rotations,&sequence,want_rows,want_tweens,0);
 want_seq[1]=sequence;mutate(&want,rotations,&sequence,want_rows,want_tweens,1);
 for(i=0;i<4;i++){
  k=(i+rotations[group])%4;index=group*4+k+5;
  calls[i].layout=want_tweens[group*4+i][0];calls[i].dx=dx[k];calls[i].dy=dy[k];
  mutate(&want,rotations,&sequence,want_rows,want_tweens,i+2);
  saved[i][0]=want.reset[index];saved[i][1]=want.expected[index];
  saved[i][2]=want.symbols[index];saved[i][3]=want.previous[index];pointers[i]=want_rows[group*4+k];
 }
 for(i=0;i<4;i++){
  short *out;int table,offset;
  k=(i+3-rotations[group])%4;index=group*4+i+5;
  want.reset[index]=saved[k][0];want.expected[index]=saved[k][1];
  want.symbols[index]=saved[k][2];want.previous[index]=(signed char)saved[k][3];want_rows[group*4+i]=pointers[k];
  out=want.layouts[1][index]->tex_idx;
  for(table=0;table<42;table++)if(out==textures[table])break;
  if(table==42){bad=1;return;}
  offset=saved[k][0]<128?saved[k][0]:(int)saved[k][0]-256;
  want_tex[table][0]=pointers[k][offset];
 }
}
int main(void)
{
 unsigned seed,rot,m,alias,g,i,j,fixtures=0;
 for(seed=0;seed<256;seed++)for(g=0;g<4;g++)for(rot=0;rot<4;rot++)for(m=0;m<8;m++)for(alias=0;alias<2;alias++){
  memset(&D_800AF834,0,sizeof D_800AF834);memset(layouts,0,sizeof layouts);
  for(i=0;i<42;i++){
   layouts[i].tex_idx=textures[alias?i/2:i];
   for(j=0;j<3;j++)textures[i][j]=(short)(0x6300+i*3+j);
  }
  for(i=0;i<21;i++){
   D_800AF834.layouts[0][i]=&layouts[i];D_800AF834.layouts[1][i]=&layouts[i];
   D_800AF834.reset[i]=(unsigned char)(seed+i*19);D_800AF834.expected[i]=(unsigned char)(seed+i*31);
   D_800AF834.symbols[i]=(unsigned char)(seed+i*53);D_800AF834.previous[i]=(signed char)(seed+i*77);
   D_800AF834.unused[i]=(unsigned char)(seed+i*17);D_800AF834.ticks[i]=(short)(seed+i*9);
   for(j=0;j<256;j++)data[i][j]=(unsigned char)(i*29+j*71+seed);
  }
  for(i=0;i<16;i++){
   D_800B3E3C[i]=data[i]+128;D_800A24D8[i][0]=&layouts[i];D_800A24D8[i][1]=&layouts[21+i];
   g_casino_seqs[i]=(short)(seed*127+i);
  }
  for(i=0;i<4;i++)D_800AF9E0[i]=(signed char)((rot+i)%4);
  D_800AF9E0[g]=rot;group=g;mode=m;step=bad=call_count=0;want=D_800AF834;
  memcpy(rotations,D_800AF9E0,sizeof rotations);memcpy(want_rows,D_800B3E3C,sizeof want_rows);memcpy(want_tweens,D_800A24D8,sizeof want_tweens);memcpy(want_tex,textures,sizeof textures);sequence=g_casino_seqs[10];
  reference();func_800871BC((unsigned char)g);
  if(bad||step!=6||call_count!=4||memcmp(&want,&D_800AF834,sizeof want)||memcmp(rotations,D_800AF9E0,sizeof rotations)||memcmp(want_rows,D_800B3E3C,sizeof want_rows)||memcmp(want_tweens,D_800A24D8,sizeof want_tweens)||memcmp(want_tex,textures,sizeof textures)||sequence!=g_casino_seqs[10]){
   printf("FAIL reel quarter seed%u group%u rotation%u mode%u alias%u\n",seed,g,rot,m,alias);return 1;
  }
  fixtures++;
 }
 printf("PASS %u reel-quarter fixtures\n",fixtures);return 0;
}
