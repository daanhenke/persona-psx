#include "../../src/casino/game/reelupdate.c"
extern int printf(const char *,...);
extern void *memset(void *,int,unsigned long);
extern int memcmp(const void *,const void *,unsigned long);
ReelState D_800AF834;
CasinoLayout *D_800A142C,*D_800A1248;
unsigned int D_800A8EA4,g_casino_money_shown,g_casino_money,g_casino_lamps;
int g_casino_win;
u_char g_casino_step,g_casino_bet;
CasinoObj D_800A16F8,D_800A14DC,D_800A156C;
CasinoPalCycle native_cycles[10];
/* The test host widens the palette pointer, so the next element is 24 bytes. */
typedef char check_cycle_size[sizeof(CasinoPalCycle)==24?1:-1];
__asm__(".set D_80099D08,native_cycles-4\n.set D_80099D20,native_cycles+24");
static CasinoLayout layouts[42];
typedef struct {
 ReelState reel;
 unsigned money,shown,winshown,lamps;
 int win;unsigned char step,bet,on[10];short busy[42];
} State;
static State want;
static int mode,bad,recording,n_events,at;
static struct {int kind,index;unsigned a;int b;} events[128];
static int layout_index(CasinoLayout *p)
{int i;for(i=0;i<42;i++)if(p==&layouts[i])return i;bad=1;return 0;}
static State get_state(void)
{
 State s;int i;memset(&s,0,sizeof s);s.reel=D_800AF834;s.money=g_casino_money;s.shown=g_casino_money_shown;s.win=g_casino_win;s.winshown=D_800A8EA4;s.lamps=g_casino_lamps;s.step=g_casino_step;s.bet=g_casino_bet;
 for(i=0;i<42;i++)s.busy[i]=layouts[i].busy;
 for(i=0;i<10;i++)s.on[i]=cycles[i].on;
 return s;
}
static void put_state(const State *s)
{
 int i;D_800AF834=s->reel;g_casino_money=s->money;g_casino_money_shown=s->shown;g_casino_win=s->win;D_800A8EA4=s->winshown;g_casino_lamps=s->lamps;g_casino_step=s->step;g_casino_bet=s->bet;
 for(i=0;i<42;i++)layouts[i].busy=s->busy[i];
 for(i=0;i<10;i++)cycles[i].on=s->on[i];
}
static void mutate(State *s,int kind,int index)
{
 if((mode&1)&&kind==1){s->step=0x70;s->money+=17;s->win-=13;}
 if((mode&1)&&kind==2){s->money+=31;s->win+=19;}
 if(kind==3){s->on[index]=1;if(mode&1)s->bet=(s->bet+1)%9;}
 if((mode&2)&&kind==4)s->lamps^=(0x15U<<index);
 if(kind>=5&&kind<=8){
  if(mode&2){s->reel.layouts[0][index]=&layouts[index+21];s->busy[index+21]=1;}
  s->reel.frames[index]=4;s->reel.delta[index]=(signed char)(index-10);
  s->reel.ticks[index]=(unsigned short)(s->reel.ticks[index]+7);
 }
 if(kind==9){
  s->busy[layout_index(s->reel.layouts[0][index])]=(mode&4)?0:1;
  if(mode&2){s->reel.frames[index]=(unsigned char)-3;s->reel.delta[index]=-17;}
 }
 if(kind==10&&(mode&4)&&index<20){
  s->reel.state[index+1]=3;s->reel.layouts[0][index+1]=&layouts[index+22];s->busy[index+22]=0;
 }
}
static void event(State *s,int kind,int index,unsigned a,int b)
{
 if(recording){if(n_events>=128){bad=1;return;}events[n_events].kind=kind;events[n_events].index=index;events[n_events].a=a;events[n_events].b=b;n_events++;}
 else{if(at>=n_events||events[at].kind!=kind||events[at].index!=index||events[at].a!=a||events[at].b!=b)bad=1;at++;}
 mutate(s,kind,index);
}
static void callback(int kind,int index,unsigned a,int b)
{State s=get_state();event(&s,kind,index,a,b);put_state(&s);}
void func_80085CF8(void){callback(1,0,0,0);}
void CasinoShowNumber(unsigned n,CasinoLayout *l,u_char width)
{callback(2,l==D_800A142C?0:l==D_800A1248?1:-1,n,width);}
void CasinoStartPalCycle(CasinoPalCycle *c)
{int i;for(i=0;i<10;i++)if(c==&cycles[i])break;if(i==10){bad=1;return;}callback(3,i,0,0);}
void CasinoStartAnim(CasinoObj *o,short frames,short rx,short ry,short rz,long tx,long ty,long tz,long sx,long sy,long sz)
{
 int i=o==&D_800A16F8?0:o==&D_800A14DC?1:o==&D_800A156C?2:-1;
 if(ry||rz||tx||ty||tz||sx||sy||sz)bad=1;callback(4,i,(unsigned)(int)rx,frames);
}
void func_80086C4C(int i){callback(5,i,0,0);}
void func_80086B88(int i){callback(6,i,0,0);}
void func_80086D40(int i){callback(7,i,0,0);}
void func_80086EEC(int i){callback(8,i,0,0);}
void func_800867A8(u_char i){callback(9,i,0,0);}
void func_800868A0(u_char i){callback(10,i,0,0);}
static int signed_byte(unsigned x){x&=255;return x<128?(int)x:(int)x-256;}
static void reference(State *s)
{
 int i,phase;
 if(s->step==113||s->step==115)event(s,1,0,0,0);
 if(s->money!=s->shown){event(s,2,0,s->money,8);s->shown=s->money;}
 if((unsigned)s->win!=s->winshown){event(s,2,1,(unsigned)s->win,6);s->winshown=(unsigned)s->win;}
 if(s->bet>0&&s->bet<=8&&!s->on[s->bet])event(s,3,s->bet,0,0);
 for(phase=0;phase<3;phase++){
  int bit=phase==0?4:phase==1?0:2;
  if(((s->lamps>>bit)&1)!=((s->lamps>>(bit+1))&1)){
   int angle=phase==0&&!(s->lamps&16)?-2048:2048;
   event(s,4,phase,(unsigned)angle,phase?16:32);
   s->lamps=(s->lamps&~(1U<<(bit+1)))|((s->lamps&(1U<<bit))<<1);
  }
 }
 if(!(s->step==60||(s->step>=112&&s->step<=115)))return;
 s->reel.header[6]=0;
 for(i=0;i<21;i++){
  int st=signed_byte(s->reel.state[i]);
  if(st>=1&&st<=3&&s->busy[layout_index(s->reel.layouts[0][i])]==0){
   if(st!=2)event(s,st==1?(i<5?5:6):(i<5?7:8),i,0,0);
   event(s,9,i,0,0);
   if(st!=2)s->reel.ticks[i]=(unsigned short)(s->reel.ticks[i]+1);
  }
  if(s->busy[layout_index(s->reel.layouts[0][i])]==1){
   int delta=signed_byte((unsigned char)s->reel.delta[i]);int frames=signed_byte(s->reel.frames[i]);
   s->reel.flags[i]=(unsigned char)(s->reel.flags[i]+delta/frames);event(s,10,i,0,0);
  }
 }
}
int main(void)
{
 static const unsigned char steps[]={0,60,112,113,114,115,255,1,111,116,128,254};
 static const unsigned char states[]={0,1,2,3,4,255,128,6};
 unsigned seed,st,bet,m,busy,i,fixtures=0;
 D_800A142C=&layouts[0];D_800A1248=&layouts[1];
 for(seed=0;seed<64;seed++)for(st=0;st<12;st++)for(bet=0;bet<12;bet++)for(m=0;m<8;m++)for(busy=0;busy<3;busy++){
  State input,actual;memset(&input,0,sizeof input);
  input.money=seed&1?0xFFFFFFF0U:seed*99;input.shown=seed&2?input.money:input.money+1;
  input.win=(int)(seed*31)-1000;input.winshown=seed&4?(unsigned)input.win:(unsigned)input.win+1;
  input.lamps=(0xACB00000U|seed);input.step=steps[st];input.bet=bet==11?255:bet;
  input.reel.header[6]=0x7E;
  for(i=0;i<21;i++){
   input.reel.layouts[0][i]=&layouts[i];input.reel.layouts[1][i]=&layouts[21+i];input.busy[i]=(busy+i)%3;input.busy[21+i]=2;
   input.reel.state[i]=states[(seed+i)%8];input.reel.frames[i]=(unsigned char)((i&1)?-7:5);input.reel.delta[i]=(signed char)(seed*5+i*17);
   input.reel.ticks[i]=(unsigned short)(65530+seed+i);input.reel.flags[i]=(unsigned char)(seed+i*73);input.reel.symbols[i]=(unsigned char)(seed+i*19);
  }
  for(i=0;i<10;i++)input.on[i]=(seed+i)&1;
  mode=m;bad=n_events=at=0;put_state(&input);want=input;recording=1;reference(&want);recording=0;func_8008634C();actual=get_state();
  if(bad||at!=n_events||memcmp(&actual,&want,sizeof want)){
   printf("FAIL reel update seed%u step%u bet%u mode%u busy%u events%d/%d\n",seed,steps[st],input.bet,m,busy,at,n_events);return 1;
  }
  fixtures++;
 }
 printf("PASS %u reel-update fixtures\n",fixtures);return 0;
}
