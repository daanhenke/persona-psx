/* Check production controller order, callback re-entry effects and by-value data. */
#include "../../src/casino/game/gridresult.c"
extern int printf(const char *, ...);
extern void *memset(void *, int, unsigned long);
extern int memcmp(const void *, const void *, unsigned long);
CasinoSymbolBlock grid_symbols;
CasinoDigitRows grid_digits;
u_char D_800A8700[4], D_800A8704, g_casino_step;
short D_800AF9E8[106], D_800A2BC8;
int g_casino_timer;
typedef struct {
    int timer;short delay,ticks;u_char player,result,complete,step;
    CasinoSymbolBlock symbols;CasinoDigitRows digits;
} State;
typedef struct {int id,arg;u_char bytes[12];} Event;
static Event actual[8],expected[8];
static int used,bad,mutation,comparison;
static State read_state(void)
{
    State s;memset(&s,0,sizeof(s));
    s.timer=g_casino_timer;s.delay=D_800A2BC8;s.ticks=grid_ticks;
    s.player=grid_player;s.result=D_800A8700[3];s.complete=D_800A8704;s.step=g_casino_step;
    s.symbols=grid_symbols;s.digits=grid_digits;return s;
}
static void write_state(State s)
{
    g_casino_timer=s.timer;D_800A2BC8=s.delay;grid_ticks=s.ticks;grid_player=s.player;
    D_800A8700[3]=s.result;D_800A8704=s.complete;g_casino_step=s.step;
    grid_symbols=s.symbols;grid_digits=s.digits;
}
static void effect(State *s,int event)
{
    int i,j;
    if(mutation==1 && event==1){s->player^=1;s->timer=16;s->ticks=0;}
    if(mutation==2 && event==2){s->player=0;s->delay=123;s->complete=1;}
    if(mutation==3 && event==3){s->timer=16;s->ticks=0;s->delay=-123;}
    if(mutation==4 && event==4){s->complete=1;s->result=comparison;s->ticks=8;}
    if(mutation==5 && event==4){s->complete=2;s->ticks=-32768;s->timer=-123;}
    if(mutation==6 && event==5){s->complete=2;s->timer=123;s->step=200;}
    if(mutation==7 && event==2)for(i=0;i<4;i++)for(j=0;j<3;j++)s->digits.digit[i][j]^=255;
}
static void event(State *s,Event *out,int id,int arg,const void *bytes,int length)
{
    const u_char *p=bytes;int i;memset(out,0,sizeof(*out));out->id=id;out->arg=arg;
    for(i=0;i<length;i++)out->bytes[i]=p[i];
    effect(s,id);
}
static void callback(int id,int arg,const void *bytes,int length)
{
    State s=read_state();if(used>=8){bad=1;return;}
    event(&s,&actual[used++],id,arg,bytes,length);write_state(s);
}
void func_8008AAF8(int index,CasinoSymbolBlock symbols){callback(1,index,&symbols,9);}
u_char func_8008C9DC(void){callback(2,0,0,0);return comparison;}
void func_8008B10C(int a,int b){if(a!=1 || b!=128)bad=1;callback(3,a,0,0);}
void func_8008AD00(u_char rows[4][3],short speed)
{
    if(rows!=grid_digits.digit || speed!=128)bad=1;
    callback(4,speed,rows,12);
}
void func_8008ABD8(u_char index,CasinoDigitRows rows){callback(5,index,&rows,12);}
static State model(State s,int *count)
{
    int n=0;
    if(s.timer==0){
        s.complete=0;event(&s,&expected[n++],1,s.player,&s.symbols,9);
        event(&s,&expected[n++],2,0,0,0);s.result=comparison;
        if(s.player==0){event(&s,&expected[n++],3,1,0,0);s.delay=16;}else s.delay=0;
    }
    if(s.timer>=s.delay){
        if(!s.ticks)event(&s,&expected[n++],4,128,&s.digits,12);
        s.ticks=s.ticks==-32768?32767:s.ticks-1;
    }
    if(s.complete==1){
        switch(s.result){case 0:s.step=116;s.timer=-1;break;case 1:s.step=114;s.timer=-1;break;}
        event(&s,&expected[n++],5,s.player,&s.digits,12);s.complete=0;
    }
    *count=n;return s;
}
int main(void)
{
    static const int timers[]={-1,0,1,15,16,17,2147483647};
    static const short ticks[]={-32768,-1,0,1,16,32767};
    static const short delays[]={-32768,0,16,32767};
    int a,b,c,p,r,d,i,j,n,fixtures=0;State initial,want,got;
    for(a=0;a<7;a++)for(b=0;b<6;b++)for(c=0;c<4;c++)for(p=0;p<3;p++)
    for(r=0;r<3;r++)for(d=0;d<3;d++)for(mutation=0;mutation<8;mutation++){
        memset(&initial,0,sizeof(initial));initial.timer=timers[a];initial.ticks=ticks[b];
        initial.delay=delays[c];initial.player=p==2?255:p;initial.result=r;
        initial.complete=d;initial.step=99;comparison=(a+b+c+p+r+d)%3;
        for(i=0;i<9;i++)((u_char*)&initial.symbols)[i]=(u_char)(fixtures+i*37);
        for(i=0;i<4;i++)for(j=0;j<3;j++)initial.digits.digit[i][j]=(u_char)(fixtures+i*97+j*13);
        D_800A8700[0]=44;D_800A8700[2]=55;D_800AF9E8[103]=1234;D_800AF9E8[105]=-1234;
        want=model(initial,&n);write_state(initial);used=bad=0;func_8008915C();got=read_state();
        if(bad || used!=n || memcmp(actual,expected,n*sizeof(Event)) || memcmp(&got,&want,sizeof(State))){
            printf("grid result failed fixture=%d mutation=%d bad=%d calls=%d expected=%d\n",fixtures,mutation,bad,used,n);return 1;
        }
        if(D_800A8700[0]!=44 || D_800A8700[2]!=55 || D_800AF9E8[103]!=1234 || D_800AF9E8[105]!=-1234)return 2;
        fixtures++;
    }
    printf("%d native grid-result fixtures passed\n",fixtures);return fixtures==36288?0:3;
}
