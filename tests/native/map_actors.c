/* Replace the fixed map address while executing the production C. */
static union { unsigned align; unsigned char bytes[160128]; } storage;
#define PERSONA_TILE_MAP storage.bytes
#include "../../src/s2d/game/mapactors.c"
extern int printf(const char *,...);
extern void *memset(void *,int,unsigned long);
extern void *memcpy(void *,const void *,unsigned long);
extern int memcmp(const void *,const void *,unsigned long);
extern void abort(void);
ActorColours D_80064FD8;
MapActor D_800B0FAC[96];
void *native_model_pointer;
__asm__(".set D_800A514C,native_model_pointer-4");
static unsigned char expected_map[160128];
static MapActor expected_actors[96];
static unsigned rng[2],calls[2],attempts[2],seed,mode;
static unsigned long long trace[2];
static ActorColours expected_colours;
static void *expected_model;
static int model_tokens[4],bad;
static void record(unsigned ref,unsigned kind,unsigned arg)
{trace[ref]=(trace[ref]^(kind*131u+arg))*1099511628211ULL;calls[ref]++;}
static void model_change(unsigned ref)
{
 unsigned i=(calls[ref]+seed)%4;
 if(ref)expected_model=&model_tokens[i];else actor_model=&model_tokens[i];
}
int VSync(int n)
{
 if(n!=-1)bad=1;record(0,0,(unsigned)n);
 D_80064FD8.colours[0]=7;D_80064FD8.colours[1]=8;D_80064FD8.colours[2]=9;
 return (int)seed;
}
void srand(unsigned n){record(0,1,n);rng[0]=n;}
static int next_random(unsigned ref)
{
 unsigned value;
 rng[ref]=rng[ref]*1664525u+1013904223u;value=(rng[ref]>>16)&32767;
 record(ref,2,value);if(mode&1)model_change(ref);
 if(calls[ref]>100000){printf("FAIL placement did not terminate\n");abort();}
 return (int)value;
}
int rand(void){return next_random(0);}
static int neighbor(unsigned ref,MapActor *a,short facing)
{
 unsigned index=(unsigned)(a-(ref?expected_actors:D_800B0FAC));
 unsigned char *map=ref?expected_map:storage.bytes;
 int result;
 if(index>=96||(facing!=1&&facing!=-1))bad=1;
 record(ref,3,index);record(ref,4,(unsigned)(int)facing);record(ref,5,a->x*256u+a->y);
 result=((a->x*3u+a->y*7u+mode+(facing==1?1:5))%13)<3;
 if(mode&2){
  a->x=(a->x+1)%200;a->y=(a->y+2)%200;
  a->unused8[0]=(unsigned char)(a->unused8[0]+3);
 }
 if(mode&4){*(unsigned*)map=(*(unsigned*)map==64)?80064:64;model_change(ref);}
 return result;
}
bool func_80094F2C(void *p,short facing){return neighbor(0,(MapActor*)p,facing)?true:false;}
static unsigned short *tile(unsigned char *map,MapActor *a)
{return (unsigned short*)(map+*(unsigned*)map+a->y*400+a->x*2);}
static void reference(void)
{
 unsigned i,first_two_directions=0;
 ActorColours saved=expected_colours;
 record(1,0,(unsigned)-1);expected_colours.colours[0]=7;expected_colours.colours[1]=8;expected_colours.colours[2]=9;
 record(1,1,seed);rng[1]=seed;
 for(i=0;i<96;){
  MapActor *a=&expected_actors[i];int d;
  attempts[1]++;
  a->colour=(unsigned char)saved.colours[next_random(1)%3];
  a->model=a->current_model=expected_model;
  d=first_two_directions>=24?(next_random(1)%2)*2+1:next_random(1)%4;
  a->direction=(unsigned char)d;a->enabled=1;
  if(d==0||d==2){
   a->x=(next_random(1)&1)|(d==0?100:102);a->last_x=a->x;
   a->y=(next_random(1)%50)*4;a->last_y=a->y;
   a->dy=d==0?1:-1;a->axis=0;
  }else{
   a->x=(next_random(1)%50)*4;a->last_y=a->x;
   a->y=(next_random(1)&3)|(d==1?60:64);a->last_x=a->y;
   a->dx=d==1?1:-1;a->axis=1;
  }
  if(*tile(expected_map,a)&0x800)continue;
  if(neighbor(1,a,1)&&neighbor(1,a,-1))continue;
  if(d<2){first_two_directions++;if(first_two_directions>=96)first_two_directions=95;}
  *tile(expected_map,a)|=0x800;i++;
 }
}
int main(void)
{
 unsigned s,m,i,k,pass=0;
 unsigned short *a;
 for(s=0;s<256;s++)for(m=0;m<8;m++){
  seed=s;mode=m;bad=0;
  memset(storage.bytes,0xA5,sizeof storage.bytes);
  *(unsigned*)storage.bytes=(s&1)?64:80064;
  for(k=0;k<2;k++){
   a=(unsigned short*)(storage.bytes+64+k*80000);
   for(i=0;i<40000;i++)a[i]=(unsigned short)(0x4200|((i*7+s+k)%19==0?0x800:0));
  }
  memcpy(expected_map,storage.bytes,sizeof expected_map);
  memset(D_800B0FAC,0xA5,sizeof D_800B0FAC);memcpy(expected_actors,D_800B0FAC,sizeof expected_actors);
  D_80064FD8.colours[0]=(signed char)(s*13);D_80064FD8.colours[1]=(signed char)(s*17+127);D_80064FD8.colours[2]=(signed char)(s*19+128);
  expected_colours=D_80064FD8;expected_model=actor_model=&model_tokens[0];
  calls[0]=calls[1]=attempts[0]=attempts[1]=0;trace[0]=trace[1]=1469598103934665603ULL;
  reference();func_80094660();
  if(bad||calls[0]!=calls[1]||trace[0]!=trace[1]||memcmp(expected_actors,D_800B0FAC,sizeof expected_actors)||memcmp(expected_map,storage.bytes,sizeof expected_map)||memcmp(&expected_colours,&D_80064FD8,sizeof expected_colours)||actor_model!=expected_model){
   printf("FAIL map actors seed%u mode%u calls%u/%u attempts%u\n",s,m,calls[0],calls[1],attempts[1]);return 1;
  }
  pass++;
 }
 printf("PASS %u complete map-actor placement fixtures\n",pass);return 0;
}
