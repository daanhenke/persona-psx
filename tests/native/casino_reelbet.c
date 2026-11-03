#include <persona/casino/casino.h>
extern int puts(const char *);
extern void func_80082E1C(void);
CasinoPalAnim D_800999C0;
CasinoLayoutDef D_800A10D8;
static CasinoLayout layout;
/* Read the PS1 flag offset; other palette fields are unused by these mocks. */
extern u_char reel_bet_palette_on __asm__("D_800999C0+0x10");
u_char g_casino_max_bet,g_casino_bet,g_casino_bet_shown,g_casino_bet_done;
u_char g_casino_quit,g_casino_step,g_casino_game;
u_int g_casino_money,g_casino_lamps;
int g_casino_timer;
short g_casino_seqs[16],g_casino_main_vab;
static int bad,mutation,n,events[32],scrolls,delta,ranges,active,end,start,cluts,last_clut,clut_y;
static void event(int e) {if(n<32) events[n]=e;else bad=1;n++;}
void CasinoStartPalAnim(CasinoPalAnim *p) {event(1);if(p!=&D_800999C0) bad=1;reel_bet_palette_on=1;}
void func_800856A0(void) {event(2);if(mutation==1){g_casino_max_bet=1;g_casino_timer=2;}}
void SsSeqStop(short seq) {event(100+seq);}
void SsSeqPlay(short seq,char mode,short loops) {
    event(200+seq);if(mode!=1 || loops!=1) bad=1;
    if(mutation==2 && seq==5){g_casino_money=200;g_casino_bet=4;}
}
u_short GetClut(int x,int y) {
    event(3);if(x!=848) bad=1;clut_y=y;
    if(mutation==3){g_casino_bet=5;g_casino_bet_shown=2;}
    return 0xABCD;
}
void CasinoQueueCluts(short first,short count,int clut) {
    event(4);if(first!=493 || count!=5) bad=1;cluts++;last_clut=clut;
}
void CasinoScroll(CasinoLayout *p,short du,short dv,short dw,short dh,short frames) {
    event(5);if(p!=&layout || dv || dw || dh || frames!=1) bad=1;scrolls++;delta=du;
}
void func_80085920(u_char a,short e,short s) {event(6);ranges++;active=a;end=e;start=s;}
void func_80085874(void) {event(7);}
void func_80087F48(u_char on) {event(8);if(on!=1) bad=1;}
void CasinoPlaySeq(short *handles,u_long *seq,short bank) {
    event(9);if(handles!=g_casino_seqs || seq!=(u_long *)0x125F54 || bank!=17) bad=1;
}
void CasinoBuildLayouts(CasinoLayoutDef *p,int mode) {event(10);if(p!=&D_800A10D8 || mode) bad=1;}
void CasinoFade(int first,int count,u_char r,u_char g,u_char b,short frames) {
    event(11);if(first || count!=558 || r || g || b || frames!=32) bad=1;
}
static void reset(void) {
    int i;
    reel_bet_palette_on=1;D_800A10D8.l=&layout;
    g_casino_max_bet=g_casino_bet=g_casino_bet_shown=g_casino_bet_done=g_casino_quit=0;
    g_casino_step=9;g_casino_game=5;g_casino_money=100;g_casino_lamps=63;g_casino_timer=0;
    g_casino_main_vab=17;for(i=0;i<16;i++) g_casino_seqs[i]=i;
    bad=mutation=n=scrolls=delta=ranges=active=end=start=cluts=last_clut=clut_y=0;
}
static int calls(const int *expected,int count) {
    int i;if(bad || n!=count) return 0;
    for(i=0;i<count;i++) if(events[i]!=expected[i]) return 0;
    return 1;
}
int main(void) {
    static const int inserted[]={2,112,212,3,4,5,6,7};
    static const int accepted[]={3,4,5,6,7,113,213,8};
    static const int accepted_sound[]={112,212,3,4,5,6,7,113,213,8};
    static const int redraw[]={3,4,5,6,7};
    static const int clear[]={4,5,6,7};
    static const int sentinel[]={4,6,7};
    static const int accept[]={113,213,8};
    static const int refund[]={9,105,205,4,10,6,7,11};
    static const int leave[]={9,11};
    static const int palette[]={1};
    reset();func_80082E1C();if(!calls(0,0)) return 1;
    reset();reel_bet_palette_on=0;func_80082E1C();if(!calls(palette,1)) return 2;
    reset();mutation=1;g_casino_timer=1;func_80082E1C();
    if(!calls(inserted,8) || g_casino_bet!=1 || g_casino_money!=99 || g_casino_bet_shown!=1 || delta!=16) return 3;
    reset();g_casino_max_bet=1;g_casino_timer=1;func_80082E1C();if(!calls(0,0) || g_casino_money!=100) return 4;
    reset();g_casino_max_bet=1;g_casino_bet=g_casino_bet_shown=7;g_casino_money=1;func_80082E1C();
    if(!calls(accepted,8) || g_casino_money || g_casino_bet!=8 || g_casino_bet_done || g_casino_step!=112 || g_casino_timer!=-1 || g_casino_lamps!=42) return 5;
    reset();g_casino_max_bet=1;g_casino_bet=g_casino_bet_shown=7;g_casino_money=2;func_80082E1C();
    if(!calls(accepted_sound,10) || g_casino_money!=1) return 6;
    reset();g_casino_bet=4;g_casino_bet_shown=1;func_80082E1C();
    if(!calls(redraw,5) || delta!=48 || clut_y!=499 || last_clut!=0xABCD || active!=1 || end!=4 || start!=1 || g_casino_bet_shown!=4) return 7;
    reset();g_casino_bet_shown=3;func_80082E1C();
    if(!calls(clear,4) || delta!=-48 || last_clut!=0x7C27 || active || end || start!=3 || g_casino_bet_shown) return 8;
    reset();g_casino_bet_shown=255;func_80082E1C();
    if(!calls(sentinel,3) || scrolls || start!=255 || g_casino_bet_shown) return 9;
    reset();g_casino_bet=g_casino_bet_shown=1;g_casino_money=0;func_80082E1C();
    if(!calls(accept,3) || g_casino_step!=112 || g_casino_timer!=-1) return 10;
    reset();g_casino_bet_done=g_casino_quit=2;func_80082E1C();if(!calls(0,0)) return 11;
    reset();g_casino_bet=g_casino_bet_shown=3;g_casino_quit=1;func_80082E1C();
    if(!calls(refund,8) || g_casino_money!=103 || g_casino_bet || g_casino_bet_shown!=3 || g_casino_lamps!=42 || g_casino_timer!=-1 || g_casino_game!=31) return 12;
    reset();g_casino_quit=1;func_80082E1C();if(!calls(leave,2) || g_casino_money!=100) return 13;
    reset();g_casino_bet=g_casino_bet_shown=3;g_casino_quit=1;mutation=2;func_80082E1C();
    if(!calls(refund,8) || g_casino_money!=204 || g_casino_bet) return 14;
    reset();g_casino_bet=3;g_casino_bet_shown=1;mutation=3;func_80082E1C();
    if(!calls(redraw,5) || clut_y!=498 || delta!=48 || end!=5 || start!=2 || g_casino_bet_shown!=5) return 15;
    reset();g_casino_bet_done=1;g_casino_lamps=55;func_80082E1C();
    if(!calls(accept,3) || g_casino_lamps!=38) return 16;
    puts("16 native reel-betting fixtures passed");return 0;
}
