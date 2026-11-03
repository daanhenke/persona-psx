#include <persona/casino/casino.h>
#include <persona/casino/grid.h>
#include <libsnd.h>
extern int printf(const char *,...);
extern void *memset(void *,int,unsigned long);
extern void *memcpy(void *,const void *,unsigned long);
extern int memcmp(const void *,const void *,unsigned long);
/* Native views use separate, correctly aligned pointer-bearing objects.
   The complete PS1 build verifies the original aliases and addresses. */
#define __asm__(address)
#include "../../src/casino/game/gridroundreset.c"
#undef __asm__
CasinoSymbolBlock D_800655B0;
CasinoDigitRows D_800655BC,D_800655C8;
u_char D_800A8700[4],D_800A8704[4],g_casino_step;
short D_800A86FC,D_800AF9E8[106],g_casino_seqs[16];
s8 D_800AFC00[3][3];
int g_casino_timer;
u_int g_casino_lamps;
CasinoCursor g_casino_cursor;
u_char grid_player;
short grid_ticks;
CasinoObj *all_objects[900];
CasinoLayoutDef choices[2],round_defs[1];
CasinoLayout *scroll_layout;
u_char *cell_objects[9][5];
static CasinoLayout layouts[3];
static CasinoObj objects[2];
static u_char cell_on[9];
typedef struct {
    int timer;unsigned lamps;short ticks,round;
    u_char player,done,slide,step;
    s8 cells[9],x,y;u_char on[9];
    CasinoSymbolBlock symbols;CasinoDigitRows digits,reset;
} State;
typedef struct {int id,a,b;u_char data[12];} Event;
enum {SCROLL,RESET,BUILD,SYMBOLS,DIGITS,PALETTE,TWEEN,STOP,PLAY,SHRINK,GROW,ROLL,ADVANCE,ANIM};
static Event events[2048];
static int recording,n,at,bad,mode,pattern;
static State read_state(void)
{
    State s;memset(&s,0,sizeof s);s.timer=g_casino_timer;s.lamps=g_casino_lamps;
    s.ticks=grid_ticks;s.round=D_800A86FC;s.player=grid_player;
    s.done=D_800A8704[0];s.slide=D_800A8704[1];s.step=g_casino_step;
    s.x=g_casino_cursor.x;s.y=g_casino_cursor.y;memcpy(s.cells,D_800AFC00,9);
    memcpy(s.on,cell_on,9);s.symbols=D_800655B0;s.digits=D_800655BC;s.reset=D_800655C8;return s;
}
static void write_state(const State *s)
{
    g_casino_timer=s->timer;g_casino_lamps=s->lamps;grid_ticks=s->ticks;D_800A86FC=s->round;
    grid_player=s->player;D_800A8704[0]=s->done;D_800A8704[1]=s->slide;g_casino_step=s->step;
    g_casino_cursor.x=s->x;g_casino_cursor.y=s->y;memcpy(D_800AFC00,s->cells,9);
    memcpy(cell_on,s->on,9);D_800655B0=s->symbols;D_800655BC=s->digits;D_800655C8=s->reset;
}
static void event(State *s,int id,int a,int b,const void *data,int size)
{
    Event e;memset(&e,0,sizeof e);e.id=id;e.a=a;e.b=b;if(data)memcpy(e.data,data,size);
    if(recording){if(n==2048){bad=1;return;}events[n++]=e;}
    else {if(at>=n||memcmp(&events[at],&e,sizeof e))bad=1;at++;}
    if(mode&1){
        if(id==SCROLL)s->player=3;
        if(id==RESET)s->timer=32;
        if(id==BUILD&&a==0)s->timer=-1;
        if(id==SYMBOLS)s->digits.digit[0][0]^=0xAA;
        if(id==DIGITS)s->symbols.rows[0].a^=0x55;
    }
    if(mode&2){
        if(id==PLAY){s->cells[pattern%9]=(pattern&1)?-1:8;s->on[(pattern+1)%9]^=1;}
        if(id==GROW||id==SHRINK){s->timer=-8;s->slide^=1;}
        if(id==ROLL){s->done=1;s->ticks=-32768;s->reset.digit[3][2]^=7;}
    }
    if(mode&4){
        if(id==ADVANCE){s->done=9;s->timer=100;}
        if(id==ANIM){s->step=99;s->lamps^=8;s->timer++;}
    }
}
static void callback(int id,int a,int b,const void *data,int size)
{State s=read_state();event(&s,id,a,b,data,size);write_state(&s);}
/* Calls without prototypes in the original controller pass promoted ints. */
void CasinoScroll(CasinoLayout *p,int du,int dv,int dw,int dh,int frames)
{if(p!=layouts+2||du||dw||dh||frames!=1)bad=1;callback(SCROLL,dv,0,0,0);}
void func_8008B428(void){callback(RESET,0,0,0,0);}
void CasinoBuildLayouts(CasinoLayoutDef *p,int mode)
{
    int which=p==round_defs?0:p==choices?1:-1;
    if(which<0||mode!=(which==0))bad=1;callback(BUILD,which,mode,0,0);
}
void func_8008AAF8(int player,CasinoSymbolBlock symbols)
{callback(SYMBOLS,player,0,&symbols,9);}
void func_8008ABD8(u8 player,CasinoDigitRows digits)
{callback(DIGITS,player,0,&digits,12);}
void func_8008AA64(int player,int active)
{if(active)bad=1;callback(PALETTE,player,0,0,0);}
void CasinoTween(CasinoLayout *p,int dx,short dy,short dz,short wait,short frames)
{int i=p==layouts?0:p==layouts+1?1:-1;if(i<0||dx!=-160||dy||dz||wait||frames!=32)bad=1;callback(TWEEN,i,0,0,0);}
void SsSeqStop(short seq){if(seq!=g_casino_seqs[9])bad=1;callback(STOP,0,0,0,0);}
void SsSeqPlay(short seq,char play,short repeat)
{if(seq!=g_casino_seqs[9]||play!=1||repeat!=1)bad=1;callback(PLAY,0,0,0,0);}
void func_8008B93C(short speed)
{if(speed!=128)bad=1;callback(SHRINK,g_casino_cursor.x,g_casino_cursor.y,0,0);}
void func_8008B870(int speed,int next)
{if(speed!=128)bad=1;callback(GROW,g_casino_cursor.x+3*g_casino_cursor.y,next,0,0);}
void func_8008AD00(u_char digits[4][3],short speed)
{if(speed!=128)bad=1;callback(ROLL,0,0,digits,12);}
void func_8008B10C(int direction,int speed)
{if(direction!=-1||speed!=128)bad=1;callback(ADVANCE,0,0,0,0);}
void CasinoStartAnim(CasinoObj *p,short frames,short rx,short ry,short rz,long tx,long ty,long tz,long sx,long sy,long sz)
{int i=p==objects?0:p==objects+1?1:-1;if(i<0||frames!=16||rx||ry||rz||tx||ty!=16||tz||sx||sy||sz)bad=1;callback(ANIM,i,0,0,0);}
static void reference(State *s)
{
    int i,zero=0;
    if(s->timer==0){
        s->ticks=0;
        if(s->player>1)event(s,SCROLL,16*(1-s->player),0,0,0);
        s->round=0;event(s,RESET,0,0,0,0);event(s,BUILD,0,1,0,0);
        for(i=0;i<s->player;i++){
            CasinoSymbolBlock symbols=s->symbols;CasinoDigitRows digits=s->digits;
            event(s,SYMBOLS,i,0,&symbols,9);event(s,DIGITS,i,0,&digits,12);event(s,PALETTE,i,0,0,0);
        }
        if(s->slide)event(s,TWEEN,1-s->player%2,0,0,0);
    }
    if(s->timer>=32)event(s,BUILD,1,0,0,0);
    if(s->timer>=0)for(i=0;i<9;i++){
        int next;
        if(s->cells[i]==0){zero++;continue;}
        if(s->on[i])continue;
        event(s,STOP,0,0,0,0);event(s,PLAY,0,0,0,0);s->x=i%3;s->y=i/3;
        next=s->cells[i];
        if(next<5){next=(next-1)%9;if(next<0)next=8;s->cells[i]=next;event(s,SHRINK,s->x,s->y,0,0);}
        else{next=(next+1)%9;s->cells[i]=next;event(s,GROW,i,next,0,0);}
    }
    if(s->done==0){
        if(s->ticks==0){CasinoDigitRows digits=s->reset;event(s,ROLL,0,0,&digits,12);}
        s->ticks=s->ticks==-32768?32767:s->ticks-1;
    }
    if(s->done==1){event(s,ADVANCE,0,0,0,0);s->done=2;}
    if(zero==9&&s->done==2){s->step=18;s->slide=0;s->timer=-16;s->lamps|=5;event(s,ANIM,0,0,0,0);event(s,ANIM,1,0,0,0);}
}
int main(void)
{
    static const int times[]={-1,0,31,32,33},players[]={0,1,2,3,255};
    static const short ticks[]={-32768,-1,0,1,32767};
    static const s8 values[]={0,1,4,5,8,9,-1,-128,127};
    State initial,want,got;int t,p,d,k,i;unsigned long cases=0;
    choices[0].l=layouts;choices[1].l=layouts+1;
    scroll_layout=layouts+2;all_objects[817]=objects;all_objects[851]=objects+1;
    for(i=0;i<9;i++)cell_objects[i][0]=&cell_on[i];g_casino_seqs[9]=123;
    for(t=0;t<5;t++)for(p=0;p<5;p++)for(d=0;d<4;d++)for(k=0;k<5;k++)
    for(pattern=0;pattern<32;pattern++)for(mode=0;mode<8;mode++){
        memset(&initial,0,sizeof initial);initial.timer=times[t];initial.player=players[p];initial.done=d==3?255:d;
        initial.ticks=ticks[k];initial.round=1234;initial.slide=pattern&1;initial.step=0xCD;initial.lamps=0xABC0;
        initial.x=-1;initial.y=-1;
        for(i=0;i<9;i++){initial.cells[i]=values[pattern<9?pattern:(pattern+i)%9];initial.on[i]=pattern<9?0:(pattern>>(i%5))&1;}
        for(i=0;i<9;i++)((u_char*)&initial.symbols)[i]=(u_char)(i*17+pattern);
        for(i=0;i<12;i++){((u_char*)&initial.digits)[i]=(u_char)(i*13+pattern);((u_char*)&initial.reset)[i]=(u_char)(i*19+pattern);}
        D_800A8700[0]=55;D_800A8700[2]=66;D_800A8700[3]=77;D_800A8704[2]=88;D_800A8704[3]=99;
        D_800AF9E8[103]=1234;D_800AF9E8[105]=-1234;g_casino_cursor.prev_x=11;g_casino_cursor.prev_y=12;
        want=initial;bad=n=at=0;recording=1;reference(&want);write_state(&initial);recording=0;func_8008A010();got=read_state();
        if(bad||at!=n||memcmp(&want,&got,sizeof want)){
            printf("FAIL grid reset case=%lu timer=%d player=%d done=%d ticks=%d pattern=%d mode=%d calls=%d/%d bad=%d\n",cases,times[t],players[p],initial.done,ticks[k],pattern,mode,at,n,bad);return 1;
        }
        if(D_800A8700[0]!=55||D_800A8700[2]!=66||D_800A8700[3]!=77||D_800A8704[2]!=88||D_800A8704[3]!=99||D_800AF9E8[103]!=1234||D_800AF9E8[105]!=-1234||g_casino_cursor.prev_x!=11||g_casino_cursor.prev_y!=12)return 2;
        cases++;
    }
    printf("PASS %lu grid reset fixtures\n",cases);return 0;
}
