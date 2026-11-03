#include <stdint.h>
#include <stdio.h>
#include <string.h>
void func_80091EB8(void);

/* Two contiguous 240-byte PS1 objects, with the original views sharing storage. */
union { double alignment; unsigned char bytes[480]; } native_pair;
__asm__(".global D_800B0724,D_800B0748,D_800B0800,D_800B0814,D_800B0838,D_800B08F0\n.set D_800B0724,native_pair+4\n"
        ".set D_800B0748,native_pair+40\n"
        ".set D_800B0800,native_pair+224\n"
        ".set D_800B0814,native_pair+244\n"
        ".set D_800B0838,native_pair+280\n"
        ".set D_800B08F0,native_pair+464\n");
short g_casino_seqs[16];
static unsigned char expected[480];
static unsigned mode, count_actual, count_expected, bad;
struct Event { int kind, arg; };
static struct Event actual_events[24], expected_events[24];
static int32_t get_i(const unsigned char *p,unsigned off)
{ int32_t v;memcpy(&v,p+off,4);return v; }
static void set_i(unsigned char *p,unsigned off,int32_t v)
{ memcpy(p+off,&v,4); }
static double get_d(const unsigned char *p,unsigned off)
{ double v;memcpy(&v,p+off,8);return v; }
static void set_d(unsigned char *p,unsigned off,double v)
{ memcpy(p+off,&v,8); }
static void perturb(unsigned char *p,unsigned event)
{
 unsigned k;
 for(k=0;k<2;k++) {
  unsigned o=k*240;
  if(mode&1) {
   set_d(p,o+152,get_d(p,o+152)+0.25*(event+1));
   set_d(p,o+176,get_d(p,o+176)-0.125*(event+k+1));
   set_d(p,o+184,get_d(p,o+184)+0.0625*(event+1));
  }
  if(mode&2) {
   set_i(p,o+8,get_i(p,o+8)+(k?7:-5));
   set_i(p,o+12,get_i(p,o+12)+((event&1)?-3:2));
  }
  if(mode&4) {
   p[o+224]^=(unsigned char)(event*29+1);
   p[o+225]^=(unsigned char)(event*17+3);
   p[o+231]^=(unsigned char)(event*13+7);
   p[o+232]^=(unsigned char)(event*11+5);
  }
 }
}
static void event(int reference,int kind,int arg)
{
 unsigned *n=reference?&count_expected:&count_actual;
 struct Event *events=reference?expected_events:actual_events;
 unsigned char *p=reference?expected:native_pair.bytes;
 if(*n>=24){bad=1;return;}
 events[*n].kind=kind;events[*n].arg=arg;
 perturb(p,*n);(*n)++;
}
void SsSeqStop(short seq){event(0,0,seq);}
void SsSeqPlay(short seq,char play,short repeat)
{ if(play!=1||repeat!=1)bad=1;event(0,1,seq); }
void func_80092394(void *p)
{
 int slot=p==native_pair.bytes?0:p==native_pair.bytes+240?1:-1;
 if(slot<0)bad=1;
 event(0,2,slot);
}
static void sound(int seq)
{event(1,0,g_casino_seqs[seq]);event(1,1,g_casino_seqs[seq]);}
static void reference(void)
{
 unsigned char *p=expected;
 int dx=get_i(p,8)-get_i(p,248),dy=get_i(p,12)-get_i(p,252);
 double a,b;
 unsigned k,o;
 if(dx<0)dx=-dx;if(dy<0)dy=-dy;
 if(dx<25 && dy<25) {
  if(dx<24 && p[231]==0) {
   sound(8);
   a=get_d(p,176);b=get_d(p,416);
   set_d(p,416,1.2*a);set_d(p,176,1.2*b);
   p[231]++;p[471]++;
  }
  if(dy<24 && dx<21) {
   sound(8);
   a=get_d(p,184);b=get_d(p,424);
   set_d(p,424,a);set_d(p,184,b);
   set_d(p,152,-get_d(p,152));set_d(p,392,-get_d(p,392));
   if(p[225])p[465]=1;else if(p[465])p[225]=1;
   if(p[232])p[472]=1;else if(p[472])p[232]=1;
  }
 }
 if(get_i(p,12)>=130)event(1,2,0);
 if(get_i(p,252)>=130)event(1,2,1);
 for(k=0;k<2;k++) {
  o=k*240;
  if((uint32_t)(get_i(p,o+8)+63)>=127 && (int8_t)p[o+224]>0) {
   sound(10);
   set_d(p,o+176,-1.0*get_d(p,o+176));
   set_d(p,o+176,0.13*get_d(p,o+176));
   set_d(p,o+152,get_d(p,o+152)+get_d(p,o+176));
  }
 }
}
int main(void)
{
 static const int xs[]={-80,-64,-63,-1,0,1,63,64,80};
 static const int ds[]={-25,-24,-23,-21,-20,-1,0,1,20,21,23,24,25};
 static const int ys[]={100,129,130,131};
 static const unsigned char flags[]={0,1,2,127,128,129,254,255};
 unsigned ix,iy,idx,idy,seed,m,k,n=0;
 if(sizeof(double)!=8 || sizeof(int)!=4) {
  puts("FAIL fixture ABI");return 1;
 }
 for(k=0;k<16;k++)g_casino_seqs[k]=(short)(-300+k*29);
 for(ix=0;ix<9;ix++)for(iy=0;iy<4;iy++)for(idx=0;idx<13;idx++)
 for(idy=0;idy<13;idy++)for(seed=0;seed<8;seed++)for(m=0;m<8;m++) {
  memset(native_pair.bytes,0xA5,sizeof native_pair.bytes);
  for(k=0;k<2;k++) {
   unsigned o=k*240;
   set_i(native_pair.bytes,o+8,xs[ix]+(k?ds[idx]:0));
   set_i(native_pair.bytes,o+12,ys[iy]+(k?ds[idy]:0));
   set_d(native_pair.bytes,o+152,((int)seed-3)*(k?-1.25:0.5));
   set_d(native_pair.bytes,o+176,seed==0?(k?-0.0:0.0):((int)seed-4)*(k?-0.125:0.25));
   set_d(native_pair.bytes,o+184,((int)seed-5)*(k?-0.0625:0.75));
   native_pair.bytes[o+224]=flags[(seed+k)%8];
   native_pair.bytes[o+225]=flags[(seed+k*3)%8];
   native_pair.bytes[o+231]=flags[(seed+k*5)%8];
   native_pair.bytes[o+232]=flags[(seed+k*7)%8];
  }
  memcpy(expected,native_pair.bytes,480);mode=m;count_actual=count_expected=bad=0;
  reference();func_80091EB8();
  if(bad||count_actual!=count_expected||memcmp(actual_events,expected_events,count_actual*sizeof *actual_events)||memcmp(native_pair.bytes,expected,480)) {
   for(k=0;k<480;k++)if(native_pair.bytes[k]!=expected[k])printf("byte%u actual%02x expected%02x\n",k,native_pair.bytes[k],expected[k]);
   printf("FAIL collision x%u y%u dx%u dy%u seed%u mode%u events%u/%u\n",ix,iy,idx,idy,seed,m,count_actual,count_expected);return 1;
  }
  n++;
 }
 printf("PASS %u preview-collision fixtures\n",n);return 0;
}
