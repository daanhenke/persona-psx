#include "../../src/casino/game/gridaccept.c"
extern int printf(const char *, ...);
extern void *memset(void *, int, unsigned long);
extern int memcmp(const void *, const void *, unsigned long);
u_char D_800A878C,D_800B0AF0,g_casino_bet_done,g_casino_quit,g_casino_step,g_casino_game;
s8 D_800AF73C[8];void *D_800A2BC8[900];
int g_casino_timer;u_long g_casino_pad_trig;u_int g_casino_lamps;
short g_casino_seqs[16],g_casino_main_vab;
u_short *g_casino_coin_item;
CasinoCursor g_casino_cursor;
static u_short coin;
static CasinoObj objects[3];static CasinoLayout number_layout;
typedef struct {
    int timer;u_int pad,lamps;u_short coin;
    u_char count,shown,bet,quit,step,game; s8 toggle;CasinoCursor cursor;
} State;
typedef struct {int kind;long value;State state;} Event;
static Event actual[12],expected[12];static int used,bad,mutation;
static State read_state(void)
{
    State s;memset(&s,0,sizeof(s));s.timer=g_casino_timer;s.pad=g_casino_pad_trig;s.lamps=g_casino_lamps;
    s.coin=coin;s.count=D_800A878C;s.shown=D_800B0AF0;s.bet=g_casino_bet_done;s.quit=g_casino_quit;
    s.step=g_casino_step;s.game=g_casino_game;s.toggle=D_800AF73C[5];s.cursor=g_casino_cursor;return s;
}
static void write_state(State s)
{
    g_casino_timer=s.timer;g_casino_pad_trig=s.pad;g_casino_lamps=s.lamps;coin=s.coin;
    D_800A878C=s.count;D_800B0AF0=s.shown;g_casino_bet_done=s.bet;g_casino_quit=s.quit;
    g_casino_step=s.step;g_casino_game=s.game;D_800AF73C[5]=s.toggle;g_casino_cursor=s.cursor;
}
static void event(State *s,Event *e,int kind,long value)
{
    memset(e,0,sizeof(*e));e->kind=kind;e->value=value;e->state=*s;
    if(mutation==1 && kind==1)s->quit=1;
    if(mutation==2 && kind==2)s->quit=0;
    if(mutation==3 && (kind==2 || kind==3 || kind==6 || kind==7))s->lamps^=5;
    if(mutation==4 && kind==4){s->count=255;s->shown=7;s->quit=2;}
    if(mutation==5 && kind==5)s->lamps^=15;
    if(mutation==6 && kind==8){s->count++;s->pad^=2048;}
}
static void callback(int kind,long value)
{
    State s=read_state();if(used>=12){bad=1;return;}event(&s,&actual[used++],kind,value);write_state(s);
}
void func_8008C8D0(void){callback(1,0);}
void CasinoStartAnim(CasinoObj *p,short frames,short rx,short ry,short rz,long tx,long ty,long tz,long sx,long sy,long sz)
{
    int kind=0;
    if(frames!=16 || ry || rz || tx || tz || sx || sy || sz)bad=1;
    if(rx==0 && ty==-16)kind=p==&objects[1]?2:p==&objects[2]?3:0;
    if(rx==2048 && ty==0)kind=p==&objects[0]?6:p==&objects[1]?7:0;
    if(!kind)bad=1;callback(kind,0);
}
void CasinoPlaySeq(short *p,u_long *address,short vab)
{
    if(p!=g_casino_seqs+3 || address!=(u_long*)0x12f00c || vab!=g_casino_main_vab)bad=1;
    callback(4,0);
}
void CasinoFade(int first,int count,u_char r,u_char g,u_char b,short frames)
{
    if(first || count!=333 || r || g || b || frames!=32)bad=1;callback(5,0);
}
void CasinoShowNumber(u_int value,CasinoLayout *p,u_char digits)
{
    if(p!=&number_layout || digits!=2)bad=1;callback(8,value);
}
static State model(State s,int *count)
{
    int n=0;
    if(s.bet==1){
        event(&s,&expected[n++],1,0);s.cursor.w=s.cursor.h=3;
        s.cursor.prev_x=s.cursor.x=0;s.cursor.prev_y=s.cursor.y=0;s.cursor.wrap_x=s.cursor.wrap_y=1;s.lamps&=~1u;
        event(&s,&expected[n++],2,0);event(&s,&expected[n++],3,0);s.timer=-1;s.bet=0;s.step=30;
    }
    if(s.quit==1){
        s.coin=(u_short)(s.count*512+35);event(&s,&expected[n++],4,0);s.lamps&=~5u;
        event(&s,&expected[n++],5,0);s.timer=-1;s.game=31;s.step=254;s.quit=0;
    }
    if((s.lamps&1)!=((s.lamps>>1)&1)){
        event(&s,&expected[n++],6,0);s.lamps=(s.lamps&~2u)|((s.lamps&1)<<1);
    }
    if(((s.lamps>>2)&1)!=((s.lamps>>3)&1)){
        event(&s,&expected[n++],7,0);s.lamps=(s.lamps&~8u)|((s.lamps&4)<<1);
    }
    if(s.count!=s.shown){event(&s,&expected[n++],8,s.count);s.shown=s.count;}
    if(s.pad&2048)s.toggle=s.toggle==0;
    *count=n;return s;
}
int main(void)
{
    int bet,quit,flags,value,t,fixtures=0,n,i;State initial,want,got;
    D_800A2BC8[783]=&objects[0];D_800A2BC8[817]=&objects[1];D_800A2BC8[851]=&objects[2];D_800A2BC8[101]=&number_layout;
    g_casino_coin_item=&coin;g_casino_main_vab=-32768;
    for(bet=0;bet<3;bet++)for(quit=0;quit<3;quit++)for(flags=0;flags<16;flags++)
    for(value=0;value<256;value+=17)for(t=0;t<4;t++)for(mutation=0;mutation<7;mutation++){
        memset(&initial,0,sizeof(initial));initial.bet=bet;initial.quit=quit;initial.lamps=0x8abc1000|flags;
        initial.count=value;initial.shown=t&1?value:255-value;initial.pad=t&2?0x800:0;
        initial.toggle=t==0?0:t==1?-128:t==2?-1:127;initial.timer=123;initial.step=17;initial.game=28;
        initial.coin=fixtures;for(i=0;i<14;i++)((u_char*)&initial.cursor)[i]=(u_char)(i*13+191);
        for(i=0;i<8;i++)D_800AF73C[i]=42;
        want=model(initial,&n);write_state(initial);used=bad=0;func_8008A628();got=read_state();
        if(bad || used!=n || memcmp(actual,expected,n*sizeof(Event)) || memcmp(&got,&want,sizeof(State))){
            printf("grid accept failed case=%d mutation=%d bad=%d\n",fixtures,mutation,bad);return 1;
        }
        for(i=0;i<8;i++)if(i!=5 && D_800AF73C[i]!=42)return 2;
        fixtures++;
    }
    printf("%d native grid-acceptance fixtures passed\n",fixtures);return fixtures==64512?0:3;
}
