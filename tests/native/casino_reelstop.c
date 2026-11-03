#include <persona/casino/reels.h>
#include <libsnd.h>
extern int printf(const char *,...);
extern void *memset(void *,int,unsigned long);
extern int memcmp(const void *,const void *,unsigned long);
extern void *memcpy(void *,const void *,unsigned long);
void func_80083398(void);
union { unsigned word; signed char bytes[5]; } native_modes;
unsigned char native_counters[8];
__asm__(".global D_800AF830,D_800AF834,D_800AF738,D_800AF73C\n"
        ".set D_800AF830,native_modes\n.set D_800AF834,native_modes+4\n"
        ".set D_800AF738,native_counters\n.set D_800AF73C,native_counters+4\n");
signed char D_800AF9E0[4];
CasinoPalAnim D_80099A20;
CasinoLayout *D_800A15CC;
CasinoObj D_800A156C;
int g_casino_timer;
u_char g_casino_step;
u_long g_casino_pad_trig;
short g_casino_seqs[16];
static CasinoLayout layouts[2];
typedef struct {
 signed char modes[5],rotations[4];unsigned char counters[8],step,on,layout;
 int timer;unsigned pad;
} State;
static unsigned mode,bad,recording,n_events,at;
static struct {int kind,arg,extra;} events[32];
static State want;
static State get_state(void)
{
 State s;memset(&s,0,sizeof s);
 memcpy(s.modes,native_modes.bytes,5);memcpy(s.rotations,D_800AF9E0,4);memcpy(s.counters,native_counters,8);
 s.timer=g_casino_timer;s.step=g_casino_step;s.pad=g_casino_pad_trig;s.on=D_80099A20.on;
 s.layout=D_800A15CC==&layouts[1];return s;
}
static void put_state(const State *s)
{
 memcpy(native_modes.bytes,s->modes,5);memcpy(D_800AF9E0,s->rotations,4);memcpy(native_counters,s->counters,8);
 g_casino_timer=s->timer;g_casino_step=s->step;g_casino_pad_trig=s->pad;D_80099A20.on=s->on;
 D_800A15CC=&layouts[s->layout];
}
static void mutate(State *s,int kind,int arg)
{
 if(mode&1) {
  if(kind==2){s->modes[arg+1]=(signed char)((s->modes[arg+1]+1)&7);s->counters[(arg+2)&7]++;}
  if(kind==3){s->pad^=0x90;s->timer+=17;}
 }
 if(kind==4)s->on=1;
 if((mode&2)&&(kind==0||kind==1)){
  s->layout^=1;s->counters[6]^=1;
  if(arg==g_casino_seqs[4])s->counters[3]^=0x80;
 }
 if((mode&4)&&kind==5){s->timer=0;s->pad^=0xA;s->counters[2]^=1;}
 if((mode&4)&&kind==6){s->layout^=1;s->counters[6]=1;}
}
static void event(State *s,int kind,int arg,int extra)
{
 if(recording){if(n_events>=32){bad=1;return;}events[n_events].kind=kind;events[n_events].arg=arg;events[n_events].extra=extra;n_events++;}
 else{if(at>=n_events||events[at].kind!=kind||events[at].arg!=arg||events[at].extra!=extra)bad=1;at++;}
 mutate(s,kind,arg);
}
static void callback(int kind,int arg,int extra)
{State s=get_state();event(&s,kind,arg,extra);put_state(&s);}
void SsSeqStop(short n){callback(0,n,0);}
void SsSeqPlay(short n,char play,short repeat)
{if(play!=1||repeat!=1)bad=1;callback(1,n,0);}
void func_80085A7C(signed char i){if(i<0||i>3)bad=1;callback(2,i,0);}
void func_80085B74(signed char i){if(i<0||i>3)bad=1;callback(3,i,0);}
void CasinoStartPalAnim(CasinoPalAnim *p){if(p!=&D_80099A20)bad=1;callback(4,0,0);}
void func_80085F08(void){callback(5,0,0);}
void CasinoTween(CasinoLayout *p,int dx,short dy,short dz,short wait,short frames)
{if(dx||dz||wait||frames!=16||(dy!=16&&dy!=-16))bad=1;callback(6,p==&layouts[0]?0:p==&layouts[1]?1:-1,dy);}
void CasinoStartAnim(CasinoObj *p,short frames,short rx,short ry,short rz,long tx,long ty,long tz,long sx,long sy,long sz)
{if(p!=&D_800A156C||frames!=16||rx||ry||rz||tx||tz||sx||sy||sz||(ty!=16&&ty!=-16))bad=1;callback(7,(int)ty,0);}
static void reference(State *s)
{
 int i,pending=1,any=0;
 if(s->timer%17==0)event(s,0,g_casino_seqs[11],0);
 event(s,1,g_casino_seqs[11],0);
 for(i=1;i<5;i++)if(s->modes[i]==4)any=1;
 if((s->pad&0xF0)&&any)for(i=1;i<5;i++)if(s->modes[i]==4)s->modes[i]=3;
 if(s->timer>=0)for(i=0;i<4;i++){event(s,2,i,0);event(s,3,i,0);}
 if(s->timer==104)for(i=1;i<5;i++)if(s->modes[i]==4)s->modes[i]=3;
 if(!s->counters[6]){
  static const int which[]={2,3,1,4},bits[]={128,32,16,64},counter[]={3,2,4,5};
  for(i=0;i<4;i++)if((s->modes[which[i]]==0||s->modes[which[i]]==6)&&(s->pad&bits[i]))s->counters[counter[i]]++;
 }
 if(!s->on)event(s,4,0,0);
 if(s->modes[1]==6&&s->modes[2]==6&&s->modes[3]==6&&s->modes[4]==6){
  for(i=1;i<5;i++)s->modes[i]=0;s->timer=0;s->step=0x71;
 }
 if(s->step!=0x71)return;
 event(s,5,0,0);
 if(s->timer==0){event(s,6,s->layout,-16);event(s,7,-16,0);}
 if(!s->counters[3]&&!s->counters[2]&&!s->counters[4]&&!s->counters[5])pending=0;
 if((s->pad&10)&&!s->counters[6]){
  event(s,0,g_casino_seqs[4],0);event(s,1,g_casino_seqs[4],0);
  s->counters[6]++;event(s,6,s->layout,16);event(s,7,16,0);
 }
 if(s->counters[6]&&!pending){s->timer=-16;s->step=0x72;s->counters[6]=s->counters[7]=0;for(i=0;i<4;i++)s->rotations[i]=0;s->modes[0]=3;}
}
int main(void)
{
 static const int timers[]={-34,-17,-16,-1,0,1,16,17,103,104,105};
 static const unsigned char values[]={0,1,3,4,5,6,127,128,255};
 unsigned seed,t,p,m,i,n=0;
 for(i=0;i<16;i++)g_casino_seqs[i]=(short)(-100+i*19);
 for(seed=0;seed<128;seed++)for(t=0;t<11;t++)for(p=0;p<64;p++)for(m=0;m<8;m++){
  State initial,actual;memset(&initial,0,sizeof initial);
  for(i=0;i<5;i++)initial.modes[i]=(seed<9)?values[seed]:values[(seed+i*3)%9];
  for(i=0;i<8;i++)initial.counters[i]=(seed&16)?values[(seed+i*5)%9]:0;
  for(i=0;i<4;i++)initial.rotations[i]=(signed char)(seed+i*31);
  initial.timer=timers[t];initial.pad=((p&15)<<4)|((p&16)?8:0)|((p&32)?2:0);
  initial.step=(seed&32)?0x71:0x70;initial.on=seed&1;initial.layout=(seed>>1)&1;
  mode=m;bad=at=n_events=0;want=initial;recording=1;reference(&want);
  recording=0;put_state(&initial);func_80083398();actual=get_state();
  if(bad||at!=n_events||memcmp(&actual,&want,sizeof want)){
   printf("FAIL reel stop seed%u timer%u pad%u mode%u calls%u/%u\n",seed,t,p,m,at,n_events);return 1;
  }n++;
 }
 printf("PASS %u reel-stop fixtures\n",n);return 0;
}
