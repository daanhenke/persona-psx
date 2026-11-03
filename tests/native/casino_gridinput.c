#include <persona/casino/casino.h>
extern int puts(const char *);
extern void func_80088A28(void);
int g_casino_timer;
u_int g_casino_lamps;
u_long g_casino_pad_trig;
u_char D_800A878C, g_casino_bet_done, g_casino_quit;
s8 D_800AF738, D_800AF73C[3], D_800A966C, D_800A9670;
short D_800AF9E8[108], g_casino_seqs[16];
static int events[8], count, bad, mutation;
static void record(int event) {
    if(count < 8) events[count] = event;
    else bad = 1;
    count++;
}
void func_8008A9DC(int x,int y,int on) {
    record(1);
    if(x || y || on != 1 || D_800AF9E8[104] || D_800AF9E8[106] ||
       D_800A966C != 1 || D_800A9670 || !(g_casino_lamps & 1)) bad=1;
    if(mutation==1) {g_casino_timer=17;g_casino_pad_trig=2;}
}
void SsSeqStop(short seq) {
    record(2); if(seq != 42) bad=1;
    if(mutation==2) g_casino_seqs[4]=73;
}
void SsSeqPlay(short seq,char mode,short loops) {
    record(3);
    if(seq != (mutation==2?73:42) || mode!=1 || loops!=1) bad=1;
    if(mutation==2) D_800A878C=0;
}
void func_8008C4D0(void) {
    record(4);
    if(mutation==3) D_800A966C=1;
}
void func_8008C16C(void) {record(5);}
static void reset(void) {
    int i;
    g_casino_timer=17;g_casino_pad_trig=0;g_casino_lamps=0xA5D0;
    D_800A878C=0;g_casino_bet_done=0;g_casino_quit=0;D_800AF738=0;
    D_800AF73C[0]=12;D_800AF73C[1]=-12;D_800AF73C[2]=0;
    D_800A966C=0;D_800A9670=0;count=bad=mutation=0;
    for(i=0;i<108;i++) D_800AF9E8[i]=29;
    for(i=0;i<16;i++) g_casino_seqs[i]=42;
}
static int fixture(int timer,int pad,int retry,int busy,int done,int quit,int pending) {
    int cancel, confirm, active, sound, flash, prompt, i, n=0;
    int expected[5];
    unsigned lamps=0xA5D0;
    reset();g_casino_timer=timer;g_casino_pad_trig=pad;
    D_800A878C=retry;D_800AF738=busy;g_casino_bet_done=done;
    g_casino_quit=quit;D_800AF73C[2]=pending;
    /* Input truth table: cancel has priority when eligible. */
    cancel=timer>16 && (pad&5) && !busy && !done;
    confirm=timer>16 && !cancel && (pad&10) && !quit && !pending;
    active=pending || confirm;sound=active && retry;flash=active && !retry;
    prompt=timer==0 && !flash;
    if(timer==0) {expected[n++]=1;lamps=(lamps&~4U)|1U|(retry?4U:0U);}
    if(sound) {expected[n++]=2;expected[n++]=3;}
    if(flash) expected[n++]=4;
    if(prompt) expected[n++]=5;
    func_80088A28();
    if(bad || count!=n) return 0;
    for(i=0;i<n;i++) if(events[i]!=expected[i]) return 0;
    if(g_casino_quit!=(quit || cancel) || g_casino_bet_done!=(done || sound) ||
       D_800A878C!=retry-(sound?1:0) || D_800AF73C[2] ||
       D_800AF73C[0]!=12 || D_800AF73C[1]!=-12 || D_800AF738!=busy ||
       D_800A9670!=flash || D_800A966C!=prompt || g_casino_lamps!=lamps) return 0;
    for(i=0;i<108;i++) {
        int zero=(i==104 && timer==0)||(i==106 && (timer==0 || flash));
        if(D_800AF9E8[i]!=(zero?0:29)) return 0;
    }
    return 1;
}
int main(void) {
    static const int times[]={-1,0,1,16,17,255};
    static const int retries[]={0,1,255};
    int t,p,r,b,d,q,s;
    for(t=0;t<6;t++) for(p=0;p<16;p++) for(r=0;r<3;r++)
    for(b=0;b<2;b++) for(d=0;d<2;d++) for(q=0;q<2;q++) for(s=-1;s<=1;s++)
        if(!fixture(times[t],p,retries[r],b,d,q,s)) return 1;
    reset();g_casino_timer=0;mutation=1;func_80088A28();
    if(bad || count!=2 || events[0]!=1 || events[1]!=4 || D_800A966C || D_800AF73C[2]) return 2;
    reset();D_800AF73C[2]=1;D_800A878C=3;mutation=2;func_80088A28();
    if(bad || count!=2 || events[0]!=2 || events[1]!=3 || D_800A878C!=255 || !g_casino_bet_done) return 3;
    reset();D_800AF73C[2]=1;mutation=3;func_80088A28();
    if(bad || count!=2 || events[0]!=4 || events[1]!=5) return 4;
    reset();D_800A966C=2;D_800A9670=-1;func_80088A28();
    if(bad || count) return 5;
    puts("6916 native grid-input fixtures passed");return 0;
}
