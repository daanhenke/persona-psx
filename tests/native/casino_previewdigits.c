#include "../../src/casino/game/previewdigits.c"
#define CasinoSpritesSetOn NativeSpritesSetOn
#include "../../src/casino/gfx/spriteset.c"
#undef CasinoSpritesSetOn
extern int printf(const char *,...);
extern void *memset(void *,int,unsigned long);
extern void *memcpy(void *,const void *,unsigned long);
extern int memcmp(const void *,const void *,unsigned long);
PreviewValues native_values;
CasinoLayout *native_layouts[2][2];
__asm__(".set D_800A9710,native_values+20\n.set D_800A2BC8,native_layouts-0x3148");
CasinoSprites g_casino_sprites;
static CasinoLayout pictures[3];
static short tex[3][24],want_tex[3][24];
static unsigned char sprites[1000],want_sprites[1000];
static PreviewValues want_values;
static int mode,bad,calls,uploads,want_calls[10][3],expected_calls,want_uploads[2];
static int signed_byte(unsigned x){x&=255;return x<128?(int)x:(int)x-256;}
static void mutate(PreviewValues *v,int i)
{
 if(mode==1 && i<9){v->current[i+1][0]^=0x97;v->old[i+1][1]+=13;}
}
void PreviewSpritesSetOn(int first,int n,int on)
{
 int i=(first-373)/4;
 if(calls>=expected_calls||first!=want_calls[calls][0]||n!=want_calls[calls][1]||on!=want_calls[calls][2])bad=1;
 calls++;
 if(first<373||first>409||n!=1||on<0||on>1)bad=1;
 NativeSpritesSetOn((short)first,(short)n,on);
 mutate(&preview_values,i);
 if(mode==2)layouts[1][0]=&pictures[2];
}
void CasinoTexLayout(CasinoLayout *p)
{
 if(uploads>=2||p!=&pictures[want_uploads[uploads]])bad=1;
 uploads++;
 if(mode==3 && uploads==1)layouts[1][0]=&pictures[2];
}
static void reference(void)
{
 int row,bank1=1;
 expected_calls=0;
 for(row=0;row<10;row++){
  int a=want_values.current[row][0],b=want_values.current[row][1];
  int olda=want_values.old[row][0],oldb=want_values.old[row][1];
  int sum=signed_byte((unsigned)(a+b));
  int previous=signed_byte((unsigned)(olda+oldb));
  int bank=row<5?0:bank1,slot=(row%5)*4;
  if(b!=oldb){want_tex[bank][slot]=signed_byte(b)-1;want_values.old[row][1]=b;}
  if(a!=olda){want_tex[bank][slot+1]=signed_byte(a)-1;want_values.old[row][0]=a;}
  if(sum!=previous){
   int shown=sum>=10;
   want_tex[bank][slot+2]=(short)((shown?sum/10:sum)+6);
   if(shown)want_tex[bank][slot+3]=(short)(sum%10+6);
   want_calls[expected_calls][0]=373+row*4;want_calls[expected_calls][1]=1;want_calls[expected_calls][2]=shown;expected_calls++;
   want_sprites[373+row*4]=shown;
   mutate(&want_values,row);
   if(mode==2)bank1=2;
  }
 }
 want_uploads[0]=0;want_uploads[1]=mode==3?2:bank1;
}
int main(void)
{
 unsigned a,b,h,m,k,r,fixtures=0;
 g_casino_sprites.on=sprites;
 for(a=0;a<256;a++)for(b=0;b<256;b++)for(h=0;h<4;h++)for(m=0;m<4;m++){
  for(r=0;r<10;r++){
   unsigned ca=(a+r*37)&255,cb=(b+r*19)&255;
   preview_values.current[r][0]=ca;preview_values.current[r][1]=cb;
   preview_values.old[r][0]=h==0?ca:h==3?(ca^128):ca+1;
   preview_values.old[r][1]=h==2?cb-1:h==3?(cb^64):cb;
  }
  for(k=0;k<3;k++){pictures[k].tex_idx=tex[k];for(r=0;r<24;r++)tex[k][r]=(short)(0x5100+k*100+r);}
  layouts[0][0]=&pictures[0];layouts[1][0]=&pictures[1];layouts[0][1]=layouts[1][1]=0;
  memset(sprites,0xA7,sizeof sprites);memcpy(want_tex,tex,sizeof tex);memcpy(want_sprites,sprites,sizeof sprites);want_values=preview_values;
  mode=m;bad=calls=uploads=0;reference();func_80093614();
  if(bad||calls!=expected_calls||uploads!=2||memcmp(tex,want_tex,sizeof tex)||memcmp(sprites,want_sprites,sizeof sprites)||memcmp(&preview_values,&want_values,sizeof preview_values)){
   printf("FAIL preview a%u b%u history%u mode%u calls%d/%d uploads%d\n",a,b,h,m,calls,expected_calls,uploads);return 1;
  }
  fixtures++;
 }
 printf("PASS %u preview-total fixtures\n",fixtures);return 0;
}
