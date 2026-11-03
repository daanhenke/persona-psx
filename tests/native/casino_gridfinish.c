#include "../../src/casino/game/gridfinish.c"
extern int printf(const char *, ...);
extern void *memset(void *, int, unsigned long);
extern int memcmp(const void *, const void *, unsigned long);
TargetGrid grid_goal;
s8 D_800AFC00[3][3];u_char D_800B4758[4][3],D_800A8700[4],D_800A8704,g_casino_step;
short D_800AF9E8[106];int g_casino_timer;
struct { u_char padding[0x468]; CasinoLayoutDef choices[2]; } D_800A2BC8;
static CasinoLayout layouts[2];
typedef struct {int timer;short ticks;u_char done,step,player;TargetGrid goal;} State;
typedef struct {int id,a,b;TargetGrid goal;} Event;
static Event actual[24],expected[24];static int used,bad,mutation;static short matched;
static State read_state(void)
{
    State s;memset(&s,0,sizeof(s));s.timer=g_casino_timer;s.ticks=grid_ticks;
    s.done=D_800A8704;s.step=g_casino_step;s.player=D_800A8700[1];s.goal=grid_goal;return s;
}
static void write_state(State s)
{
    g_casino_timer=s.timer;grid_ticks=s.ticks;D_800A8704=s.done;g_casino_step=s.step;
    D_800A8700[1]=s.player;grid_goal=s.goal;
}
static void event(State *s,Event *e,int id,int a,int b,TargetGrid *goal)
{
    memset(e,0,sizeof(*e));e->id=id;e->a=a;e->b=b;if(goal)e->goal=*goal;
    if(mutation==1 && id==1){s->timer=32;s->player^=1;}
    if(mutation==2 && id==2){s->timer=32;s->ticks=5;s->done=1;}
    if(mutation==3 && id==3)s->timer=31;
    if(mutation==4 && id==4 && a==1 && b==1){s->done=1;s->timer=33;}
    if(mutation==5 && id==5){s->done=1;s->ticks=0;s->goal.cells[1][1]^=127;}
    if(mutation==6 && id==6){s->done=2;s->timer=-1;}
    if(mutation==7 && id==7){s->done=1;s->ticks=16;}
    if(mutation==7 && id==8){s->done=2;s->step=99;s->timer=800;}
}
static void callback(int id,int a,int b,TargetGrid *goal)
{
    State s=read_state();if(used>=24){bad=1;return;}event(&s,&actual[used++],id,a,b,goal);write_state(s);
}
void func_8008A9DC(int texture,short distance,short frames)
{
    if(texture!=3 || distance!=-128 || frames!=32)bad=1;callback(1,0,0,0);
}
void CasinoTween(CasinoLayout *p,int dx,short dy,short mode,short unused,short frames)
{
    int choice=p==layouts?0:p==layouts+1?1:-1;
    if(choice<0 || dx!=-160 || dy || mode || unused || frames!=32)bad=1;
    callback(2,choice,0,0);
}
void CasinoSpritesSetOn(short first,short count,int on)
{if(first!=293 || count!=40 || on)bad=1;callback(3,0,0,0);}
void func_8008A974(int x,int y){callback(4,x,y,0);}
short func_8008CBF8(s8 cells[3][3],TargetGrid goal)
{if(cells!=D_800AFC00)bad=1;callback(5,0,0,&goal);return matched;}
u_char func_8008C9DC(void){callback(6,0,0,0);return 255;}
void func_8008AD00(u_char rows[4][3],short speed)
{if(rows!=D_800B4758 || speed!=128)bad=1;callback(7,0,0,0);}
void func_8008D120(void){callback(8,0,0,0);}
static State model(State s,int *count)
{
    int n=0,x,y,have_match=0;
    if(!s.timer){event(&s,&expected[n++],1,0,0,0);event(&s,&expected[n++],2,1-(s.player%2),0,0);s.ticks=0;s.done=0;}
    if(s.timer==32){event(&s,&expected[n++],3,0,0,0);for(y=0;y<3;y++)for(x=0;x<3;x++)event(&s,&expected[n++],4,x,y,0);}
    if(s.timer>=32){event(&s,&expected[n++],5,0,0,&s.goal);have_match=matched==9;event(&s,&expected[n++],6,0,0,0);}
    if(have_match){if(!s.ticks)event(&s,&expected[n++],7,0,0,0);s.ticks=s.ticks==-32768?32767:s.ticks-1;}
    if(s.done==1){event(&s,&expected[n++],3,0,0,0);event(&s,&expected[n++],8,0,0,0);s.done=0;s.step=119;s.timer=-1;}
    *count=n;return s;
}
int main(void)
{
    static const int times[]={-1,0,31,32,33,2147483647};static const short ticks[]={-32768,-1,0,1,32767};
    static const short results[]={-32768,-1,0,8,9,10,32767};
    int a,b,d,p,r,i,n,fixtures=0;State initial,want,got;
    grid_choices[0].l=layouts;grid_choices[1].l=layouts+1;
    for(a=0;a<6;a++)for(b=0;b<5;b++)for(d=0;d<3;d++)for(p=0;p<3;p++)for(r=0;r<7;r++)for(mutation=0;mutation<8;mutation++){
        memset(&initial,0,sizeof(initial));initial.timer=times[a];initial.ticks=ticks[b];initial.done=d;initial.step=117;initial.player=p==2?255:p;matched=results[r];
        for(i=0;i<9;i++)((u_char*)&initial.goal)[i]=(u_char)(fixtures+i*37);
        D_800A8700[0]=44;D_800A8700[2]=55;D_800A8700[3]=66;D_800AF9E8[103]=1234;D_800AF9E8[105]=-1234;
        memset(D_800AFC00,0x55,sizeof(D_800AFC00));memset(D_800B4758,0x66,sizeof(D_800B4758));
        want=model(initial,&n);write_state(initial);used=bad=0;func_80089C04();got=read_state();
        if(bad || used!=n || memcmp(actual,expected,n*sizeof(Event)) || memcmp(&got,&want,sizeof(State))){
            printf("grid finish failed fixture=%d mutation=%d bad=%d calls=%d expected=%d\n",fixtures,mutation,bad,used,n);return 1;
        }
        if(D_800A8700[0]!=44 || D_800A8700[2]!=55 || D_800A8700[3]!=66 || D_800AF9E8[103]!=1234 || D_800AF9E8[105]!=-1234)return 2;
        for(i=0;i<9;i++)if(((u_char*)D_800AFC00)[i]!=0x55)return 3;
        for(i=0;i<12;i++)if(((u_char*)D_800B4758)[i]!=0x66)return 4;
        fixtures++;
    }
    printf("%d native grid-finish fixtures passed\n",fixtures);return fixtures==15120?0:5;
}
