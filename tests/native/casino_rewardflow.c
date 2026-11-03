/* Drop only PS1 linker aliases. Execute the production C with host-sized data. */
#define __asm__(symbol)
#define func_800915FC real_reward_blink
#include "../../src/casino/game/rewardblink.c"
#undef func_800915FC
#include "../../src/casino/game/rewardselect.c"
#include "../../src/casino/game/rewardreveal.c"
#undef __asm__
extern int puts(const char *);
extern int printf(const char *, ...);
CasinoRewardData g_casino_reward_data;
CasinoPalAnim D_80099CA8;
u_char g_casino_reward_palette_on, D_800AAE9C[16], g_casino_wrapped, g_casino_step;
int g_casino_timer;
u_long g_casino_pad_trig;
short g_casino_seqs[16], g_casino_vab, g_casino_main_vab;
CasinoLayout *D_800A2BC8[3024], *result_layout, *result_frame;
static CasinoLayout layouts[8];
static const int layout_slots[] = {0x1C70/4, 0x2DB8/4, 0x2DC0/4, 0x2EF0/4, 2947, 2965, 2986, 0x2F3C/4};
static const u_int amounts[] = {0,9,10,99,100,999,1000,9999,10000,65535,0xffffffffU};
typedef struct { int kind,a,b,c; } Event;
static Event expected[80];
static int expected_n, actual_n, bad, mutation_at, mutation_kind, predicting;
static int rt, rs, rm, rc, rp, rw, rflag, ron;

static void mutate(int number) {
    if (number != mutation_at) return;
    if (predicting) {
        if (mutation_kind == 0) rt = 16;
        else if (mutation_kind == 1) { rc = rc == 12 ? 2 : rc + 1; rm = (rm + 1) % 3; }
        else { rt = 17; rp = 32; }
    } else {
        if (mutation_kind == 0) g_casino_timer = 16;
        else if (mutation_kind == 1) {
            g_casino_reward_data.dice[2] = g_casino_reward_data.dice[2] == 12 ? 2 : g_casino_reward_data.dice[2] + 1;
            D_800AAE9C[2] = (D_800AAE9C[2] + 1) % 3;
        } else { g_casino_timer = 17; g_casino_pad_trig = 32; }
    }
}
static void event(int kind,int a,int b,int c) {
    Event e = {kind,a,b,c}; int n;
    if (predicting) { n=expected_n++; if(n>=80) {bad=1;return;} expected[n]=e; }
    else { n=actual_n++; if(n>=expected_n || expected[n].kind!=kind || expected[n].a!=a || expected[n].b!=b || expected[n].c!=c) bad=1; }
    mutate(n);
}
void CasinoStartPalAnim(CasinoPalAnim *p) { if(p!=&D_80099CA8) bad=1; event(1,0,0,0); }
void CasinoQueueCluts(short first,short count,int clut) { event(2,first,count,clut); }
void CasinoSpritesSetOn(short first,short count,int on) { event(3,first,count,on); }
void CasinoTween(CasinoLayout *p,int dx,short dy,short dw,short dh,short frames) {
    int i; for(i=0;i<8;i++) if(p==&layouts[i]) break;
    if(i==8 || dx || dw || dh || frames!=16) bad=1;
    event(4,i,dy,frames);
}
void CasinoPlaySeq(short *slot,u_long *seq,short vab) {
    int i; for(i=0;i<16;i++) if(slot==&g_casino_seqs[i]) break;
    if(i==16) bad=1;
    event(5,i,(int)(unsigned long)seq,vab);
}
void SsSeqStop(short handle) { event(6,handle,0,0); }
void SsSeqPlay(short handle,char mode,short repeat) { event(7,handle,mode,repeat); }
void SsSeqSetVol(short handle,short left,short right) { event(8,handle,left,right); }
void CasinoCloseSeq(short *p) { if(p!=g_casino_seqs) bad=1; event(9,0,0,0); }
void func_80091544(int n) { event(10,n,0,0); }
void func_800915FC(void) { event(11,0,0,0); }

static void panel(int id,int dy) { event(4,id,dy,16); }
static void result_panels(int dy) {
    if(rm==0) { panel(1,dy);panel(2,dy); }
    else if(rm==1) panel(3,dy);
    panel(4,dy); panel(5,dy); panel(6,dy);
}
static void show_result(void) {
    event(2,rc+62,1,0x7CA5);
    event(3,rc*4+67,4,1);
    event(3,rc*2+115,2,1);
    event(3,rc*2+137,2,1);
}
static void reference_blink(void) {
    int digits; u_int value;
    if(!ron) event(1,0,0,0);
    if(rt%8==0) event(2,rc+62,1,0x7CA5);
    if(rt%8==4) event(2,rc+62,1,0x7CE5);
    if(rt%32==0) {
        event(3,rc*4+67,4,0); event(3,rc*2+115,2,0); event(3,rc*2+137,2,0);
    }
    if(rt%32==16) {
        value=g_casino_reward_data.values[rc];
        digits=value<10?1:value<100?2:value<1000?3:value<10000?4:0;
        if(digits) event(3,rc*4+71-digits,digits,1);
        event(3,rc*2+115,2,1); event(3,rc*2+137,2,1);
    }
}
static void reference_reveal(void) {
    u_int value;
    if(rt==0) {panel(0,240);panel(7,208);}
    if(rt==16) {
        show_result();event(3,35,8,1);
        value=g_casino_reward_data.values[rc];
        if(value<1000) event(3,rc*4+67,value<100?2:1,0);
        rs=49;rt=-1;
    }
}
static void reference_select(void) {
    int selected=0;u_int value;
    if(rt>0) event(11,0,0,0);
    if(rt==0) {
        event(5,0,0x13AA88,7);panel(0,-80);result_panels(-208);event(10,rc,0,0);
    }
    if(rt==16 && rm==1) event(5,1,0x13B0BC,9);
    if(rt==62 && rm==1) event(5,0,0x13AB54,7);
    if(rt>16) {
        if((rp&96)==32) {rflag=1;selected=1;rs=51;rt=-1;}
        else if((rp&96)==64) {rs=49;rt=-1;}
    }
    if(rs!=41) {
        event(6,104,0,0);event(7,104,1,1);panel(0,240);
        if(rm==0) {panel(1,208);panel(2,208);}
        else if(rm==1) {
            event(8,115,127,127);if(rw) event(6,115,0,0);
            event(7,115,1,1);event(9,0,0,0);panel(3,208);
        }
        panel(4,208);panel(5,208);panel(6,208);show_result();
        value=g_casino_reward_data.values[rc];
        if(value<1000) event(3,rc*4+67,value<10?3:value<100?2:1,0);
        rm=selected;
    }
}
static int check(int routine,int frame,int mode,int pad,int wrapped,int step,int choice,int amount,int on) {
    int i; rt=frame;rm=mode;rp=pad;rw=wrapped;rs=step;rc=choice;ron=on;rflag=77;
    for(i=0;i<13;i++) g_casino_reward_data.values[i]=amounts[(amount+i-choice+22)%11];
    for(i=0;i<84;i++) g_casino_reward_data.reserved[i]=0x5A;
    g_casino_reward_data.dice[0]=5;g_casino_reward_data.dice[1]=6;
    for(i=0;i<16;i++) {D_800AAE9C[i]=77;g_casino_seqs[i]=100+i;}
    g_casino_vab=7;g_casino_main_vab=9;
    expected_n=actual_n=bad=0;predicting=1;
    if(routine==0) reference_blink();else if(routine==1) reference_reveal();else reference_select();
    predicting=0;g_casino_timer=frame;D_800AAE9C[2]=mode;g_casino_pad_trig=pad;
    g_casino_wrapped=wrapped;g_casino_step=step;g_casino_reward_data.dice[2]=choice;g_casino_reward_palette_on=on;
    if(routine==0) real_reward_blink();else if(routine==1) func_8008DF68();else func_8008E0C0();
    if(bad || actual_n!=expected_n || g_casino_timer!=rt || g_casino_step!=rs || D_800AAE9C[2]!=rm || D_800AAE9C[9]!=rflag || g_casino_reward_data.dice[2]!=rc || g_casino_pad_trig!=(u_long)rp) {
        printf("reward failure routine=%d frame=%d mode=%d pad=%d step=%d choice=%d amount=%d mutation=%d/%d events=%d/%d\n",routine,frame,mode,pad,step,choice,amount,mutation_at,mutation_kind,actual_n,expected_n);return 1;
    }
    for(i=0;i<84;i++) if(g_casino_reward_data.reserved[i]!=0x5A)return 2;
    for(i=0;i<13;i++) if(g_casino_reward_data.values[i]!=amounts[(amount+i-choice+22)%11]) return 3;
    for(i=0;i<16;i++) if(i!=2 && i!=9 && D_800AAE9C[i]!=77)return 4;
    if(g_casino_reward_data.dice[0]!=5 || g_casino_reward_data.dice[1]!=6)return 5;
    return 0;
}
int main(void) {
    const int frames[]={-33,-32,-16,-1,0,1,4,8,15,16,17,31,32,48,61,62,63};
    const int modes[]={0,1,2}, pads[]={0,32,64,96}, steps[]={41,49};
    int f,c,a,on,m,p,w,s,r,k,count=0;
    for(k=0;k<8;k++)D_800A2BC8[layout_slots[k]]=&layouts[k];
    result_layout=&layouts[0];result_frame=&layouts[7];
    mutation_at=-1;mutation_kind=0;
    for(r=0;r<2;r++)for(f=0;f<17;f++)for(c=2;c<=12;c++)for(a=0;a<11;a++)for(on=0;on<2;on++) {
        if(check(r,frames[f],0,0,0,41,c,a,on))return 1;count++;
    }
    for(f=3;f<17;f++)for(m=0;m<3;m++)for(p=0;p<4;p++)for(w=0;w<2;w++)for(s=0;s<2;s++)for(a=0;a<11;a++) {
        if(check(2,frames[f],modes[m],pads[p],w,steps[s],2+a,a,1))return 2;count++;
    }
    for(r=0;r<3;r++)for(f=3;f<17;f++)for(mutation_at=0;mutation_at<24;mutation_at++)for(mutation_kind=0;mutation_kind<3;mutation_kind++) {
        if(check(r,frames[f],mutation_at%3,32,1,41,2+mutation_at%11,mutation_at%11,0))return 3;count++;
    }
    printf("%d native reward-flow fixtures passed\n",count);return 0;
}
