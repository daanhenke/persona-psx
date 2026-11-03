/* The PS1 VECTOR ABI is 32-bit even on a 64-bit Linux test host. */
#include <decomp/types.h>
#define VECTOR PsyqHostVector
#include <libgte.h>
#undef VECTOR
typedef struct { int vx,vy,vz,pad; } VECTOR;
#include "../../src/casino/game/previewinit2.c"
extern int printf(const char *,...);
extern void *memset(void *,int,unsigned long);
extern int memcmp(const void *,const void *,unsigned long);
PreviewState pair;
unsigned char D_800B08F0[16];
CasinoModel native_model;
__asm__(".set D_800A2BC8,native_model-14836");
static PreviewState want;
static unsigned char want_flags[16];
static unsigned mode,step,bad;
static int same(CasinoDoubleVector a,CasinoDoubleVector b)
{ return a.x==b.x && a.y==b.y && a.z==b.z; }
static void perturb(PreviewState *p,unsigned n)
{
 if(mode&1){p->r.x+=3+n;p->r.y-=7;p->r.z+=11;}
 if(mode&2){p->p.x-=13;p->p.y+=17+n;p->p.z+=19;}
 if(mode&4){p->s.x+=0.25;p->s.y-=0.125;p->s.z+=0.0625;}
}
static void after_call(void)
{ perturb(&want,step);perturb(&pair,step);step++; }
void func_80092C44(CasinoDoubleVector v,short *out)
{
 short *expected=step==0?&want.current.rot.vx:&want.previous.rot.vx;
 short *actual=step==0?&pair.current.rot.vx:&pair.previous.rot.vx;
 if(step>1||out!=actual||!same(v,want.r))bad=1;
 out[0]=(int)v.x;out[1]=(int)v.y;out[2]=(int)v.z;
 expected[0]=(int)want.r.x;expected[1]=(int)want.r.y;expected[2]=(int)want.r.z;
 after_call();
}
void func_80092CB0(CasinoDoubleVector v,int *out)
{
 int *expected=step==2?&want.current.trans.vx:&want.previous.trans.vx;
 int *actual=step==2?&pair.current.trans.vx:&pair.previous.trans.vx;
 if(step<2||step>3||out!=actual||!same(v,want.p))bad=1;
 out[0]=(int)v.x;out[1]=(int)v.y;out[2]=(int)(762.0-v.z);
 expected[0]=(int)want.p.x;expected[1]=(int)want.p.y;expected[2]=(int)(762.0-want.p.z);
 after_call();
}
void func_80092D34(CasinoDoubleVector v,int *out)
{
 int *expected=step==4?&want.current.scale.vx:&want.previous.scale.vx;
 int *actual=step==4?&pair.current.scale.vx:&pair.previous.scale.vx;
 if(step<4||step>5||out!=actual||!same(v,want.s))bad=1;
 out[0]=(int)(4096*v.x);out[1]=(int)(4096*v.y);out[2]=(int)(4096*v.z);
 expected[0]=(int)(4096*want.s.x);expected[1]=(int)(4096*want.s.y);expected[2]=(int)(4096*want.s.z);
 after_call();
}
void func_80093B7C(CasinoXform *p)
{
 if(step!=6||p!=&pair.current||memcmp(p,&want.current,sizeof *p))bad=1;
 p->rot.vy=want.current.rot.vy=(short)(-300+mode);
 p->trans.vz=want.current.trans.vz=12345;
 p->scale.vx=want.current.scale.vx=-4096;
 after_call();
}
void func_80092DEC(CasinoModel *p,SVECTOR r,VECTOR t,VECTOR s)
{
 if(step!=7||p!=models||memcmp(&r,&want.current.rot,sizeof r)||memcmp(&t,&want.current.trans,sizeof t)||memcmp(&s,&want.current.scale,sizeof s))bad=1;
 step++;
}
int main(void)
{
 unsigned seed,m,i,count=0;
 if(sizeof(VECTOR)!=16||sizeof(CasinoXform)!=40){printf("FAIL PS1 fixture ABI\n");return 1;}
 for(seed=0;seed<256;seed++)for(m=0;m<8;m++){
  memset(&pair,(int)seed,sizeof pair);want=pair;
  for(i=0;i<16;i++)D_800B08F0[i]=want_flags[i]=(unsigned char)(seed+i*17);
  want.r.x=want.r.y=want.r.z=0;
  want.p.x=-24;want.p.y=-100;want.p.z=634;
  want.s.x=want.s.y=want.s.z=1;
  want_flags[0]=want_flags[1]=want_flags[2]=want_flags[7]=0;want_flags[8]=1;
  mode=m;step=bad=0;func_80092910();
  if(bad||step!=8||memcmp(&pair,&want,sizeof pair)||memcmp(D_800B08F0,want_flags,sizeof want_flags)){
   printf("FAIL preview initializer seed%u mode%u step%u\n",seed,m,step);return 1;
  }
  count++;
 }
 printf("PASS %u preview-initialization fixtures\n",count);return 0;
}
