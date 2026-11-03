static union { long align; unsigned char bytes[4096]; } scratch_storage;
#define PERSONA_RING_SCRATCH ((RingScratch *)scratch_storage.bytes)
#define memmove RingCopy
#include "../../src/s2d/game/mapring.c"
#undef memmove
extern int printf(const char *,...);
extern void *memset(void *,int,unsigned long);
extern int memcmp(const void *,const void *,unsigned long);
RingPackets D_800B5F5C;
__typeof__(D_800A566C) D_800A566C;
int g_draw_side;
static RingPackets expected_packets;
static DVECTOR points[65],centres[4];
static GsOT ot;
static GsOT_TAG tags[2];
static POLY_F4 *back;
static int mode,trigmode,seed,frame,side,bad,trigs,copies,adds,setups,light;
static int wave(int a)
{
 unsigned u=(unsigned)a&4095;
 if(trigmode==1)return (a&1)?4096:-4096;
 if(trigmode==2)return (int)((u*97+(unsigned)seed*31)%8193)-4096;
 return u<2048?(int)u*4-4096:12288-(int)u*4;
}
static int trig(int kind,int angle)
{
 int call=trigs++,expect,value,n;
 if(call<128){n=call/2;expect=n*64;if(kind!=(call&1))bad=1;}
 else if(call<192){n=(call-128)/2;expect=n*199+frame*96+((call&1)?2048:0);if(kind)bad=1;}
 else {expect=frame*64;if(kind||call!=192)bad=1;n=0;}
 if(angle!=expect)bad=1;
 value=wave(expect+(kind?1024:0));
 if(call<128){if(kind)points[n].vy=(short)((unsigned)(value*15)>>9);else points[n].vx=(short)((unsigned)(value*5)>>6);}
 else if(call<192){int delta=((value+4096)/2)/64;if(call&1)points[63-n].vx=(short)(points[63-n].vx+delta);else points[n].vx=(short)(points[n].vx-delta);}
 else light=((value+4096)/2)*56/4096;
 if(mode==1 && call%11==3){frame+=17;ring_frame+=17;}
 return value;
}
int rsin(int angle){return trig(0,angle);}
int rcos(int angle){return trig(1,angle);}
void SetPolyG3(POLY_G3 *p)
{
 if(p!=&PERSONA_RING_SCRATCH->poly||setups++)bad=1;
 memset(p,0x5A,sizeof(*p));p->code=0x30;p->tag=0x12345678;
}
void SetPolyF4(POLY_F4 *p)
{
 if(adds!=64||p!=&D_800B5F5C.back[side]||setups++!=3)bad=1;
 back=p;memset(p,0x5A,sizeof(*p));p->code=0x28;p->tag=0xABCDEF;
}
void SetSemiTrans(void *p,int on)
{
 if(on!=1)bad=1;
 if(setups==1){if(p!=&PERSONA_RING_SCRATCH->poly)bad=1;PERSONA_RING_SCRATCH->poly.code|=2;}
 else if(setups==4){if(p!=back)bad=1;back->code|=2;if(mode==3){side^=1;g_draw_side^=1;frame+=31;ring_frame+=31;}}
 else bad=1;
 setups++;
}
void SetShadeTex(void *p,int on)
{
 if(on)bad=1;
 if(setups==2){if(p!=&PERSONA_RING_SCRATCH->poly)bad=1;}
 else if(setups==5){if(p!=back)bad=1;}
 else bad=1;
 setups++;
}
void *RingCopy(void *dst,const void *src,unsigned int bytes)
{
 const POLY_G3 *p=src;unsigned k;int i=copies;
 if(i>=64||trigs!=192||src!=PERSONA_RING_SCRATCH||bytes!=sizeof(POLY_G3)||dst!=&D_800B5F5C.triangles[side][i])bad=1;
 points[64]=points[0];
 if(memcmp(points,PERSONA_RING_SCRATCH->points,sizeof points))bad=1;
 if(p->x0!=points[i].vx||p->y0!=points[i].vy||p->x2!=points[i+1].vx||p->y2!=points[i+1].vy||p->x1!=centres[i/16].vx||p->y1!=centres[i/16].vy)bad=1;
 if(p->tag!=0x12345678||p->code!=0x32||p->r0||p->g0||p->b0||p->r1!=255||p->g1!=207||p->b1!=175||p->r2||p->g2||p->b2)bad=1;
 expected_packets.triangles[side][i]=*p;
 for(k=0;k<bytes;k++)((unsigned char*)dst)[k]=((const unsigned char*)src)[k];
 copies++;
 if(mode==1){side^=1;g_draw_side^=1;}
 return dst;
}
void AddPrim(void *head,void *packet)
{
 if(head!=ot.org)bad=1;
 if(adds<64){
  if(packet!=&D_800B5F5C.triangles[side][adds]||copies!=adds+1)bad=1;
  if(mode==2){int n=(adds+1)/16;if(n<4){centres[n].vx+=3;D_800A566C.centre[n].vx+=3;}}
 }else if(adds==64){
  POLY_F4 *p=packet;
  if(p!=back||trigs!=193||p->code!=0x2A||p->tag!=0xABCDEF||p->r0!=light+24||p->g0!=light+8||p->b0!=light)bad=1;
  if(p->x0!=-256||p->x1!=256||p->x2!=-256||p->x3!=256||p->y0!=-120||p->y1!=-120||p->y2!=120||p->y3!=120)bad=1;
  expected_packets.back[p-D_800B5F5C.back]=*p;
 }else if(adds==65){if(packet!=&D_800B5F5C.modes[side])bad=1;}
 else bad=1;
 adds++;
 if(mode==2){side^=1;g_draw_side^=1;}
 if(mode==3)ot.org=&tags[adds&1];
}
void SetDrawMode(DR_MODE *p,int dfe,int dtd,int page,RECT *tw)
{
 if(adds!=65||p!=&D_800B5F5C.modes[side]||dfe||dtd||page!=32||tw)bad=1;
 memset(p,0x39,sizeof(*p));expected_packets.modes[side]=*p;
 if(mode==1){side^=1;g_draw_side^=1;}
}
int main(void)
{
 const int frames[]={-100000,-1,0,1234,100000};unsigned f,m,t,d,s,k,fixtures=0;
 for(s=0;s<64;s++)for(f=0;f<5;f++)for(d=0;d<2;d++)for(m=0;m<4;m++)for(t=0;t<3;t++){
  mode=m;trigmode=t;seed=s;frame=ring_frame=frames[f];side=g_draw_side=d;
  for(k=0;k<4;k++){centres[k].vx=(short)(s*701+k*3277);centres[k].vy=(short)(s*-559-k*719);D_800A566C.centre[k]=centres[k];}
  memset(&D_800B5F5C,0xA7,sizeof D_800B5F5C);expected_packets=D_800B5F5C;memset(scratch_storage.bytes,0xD3,sizeof scratch_storage.bytes);
  ot.org=&tags[0];bad=trigs=copies=adds=setups=0;func_80096A38(&ot);
  if(bad||trigs!=193||copies!=64||adds!=66||setups!=6||side!=g_draw_side||frame!=ring_frame||memcmp(&D_800B5F5C,&expected_packets,sizeof expected_packets)){
   printf("FAIL ring seed%u frame%u side%u mode%u trig%u bad%d calls%d/%d/%d/%d\n",s,f,d,m,t,bad,trigs,copies,adds,setups);return 1;}
  for(k=sizeof(RingScratch);k<sizeof scratch_storage.bytes;k++)if(scratch_storage.bytes[k]!=0xD3)return 2;
  fixtures++;
 }
 printf("PASS %u triangle-ring fixtures\n",fixtures);return 0;
}
