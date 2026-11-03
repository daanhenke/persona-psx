/* Replace only the fixed sequence address and PS1 state alias. */
static short alert_sequence;
#define PERSONA_ALERT_SEQUENCE (&alert_sequence)
#define __asm__(symbol)
#include "../../src/s2d/game/scenealert.c"
#undef __asm__
extern int printf(const char *, ...);
S2dFade g_fade;
int D_800B56EC,scene_tick;
typedef struct {int kind,a,b,c,r,g,blue;} Event;
static Event expected[8];
static int predicting,expected_n,actual_n,bad,change_at,change_kind;
static int rt,rm,rh,rr,rg,rb;
static void event(int kind,int a,int b,int c)
{
    Event e; int n;
    e.kind=kind;e.a=a;e.b=b;e.c=c;
    e.r=predicting?rr:g_fade.rgb[0];e.g=predicting?rg:g_fade.rgb[1];e.blue=predicting?rb:g_fade.rgb[2];
    if(predicting){n=expected_n++;if(n>=8){bad=1;return;}expected[n]=e;}
    else {n=actual_n++;if(n>=expected_n || expected[n].kind!=kind || expected[n].a!=a || expected[n].b!=b || expected[n].c!=c || expected[n].r!=e.r || expected[n].g!=e.g || expected[n].blue!=e.blue)bad=1;}
    if(n!=change_at)return;
    if(predicting) {
        if(change_kind==0){rt=3;rm=2;}
        else if(change_kind==1)rh=-123;
        else {rr=7;rg=8;rb=9;rt=-1;rm=0;}
    }else{
        if(change_kind==0){scene_tick=3;D_800B56EC=2;}
        else if(change_kind==1)alert_sequence=-123;
        else {g_fade.rgb[0]=7;g_fade.rgb[1]=8;g_fade.rgb[2]=9;scene_tick=-1;D_800B56EC=0;}
    }
}
void GsSetAmbient(long r,long g,long b){event(r==0x555?1:2,(int)r,(int)g,(int)b);}
void SsSeqStop(short h){event(3,h,0,0);}
void SsSeqSetVol(short h,short l,short r){event(4,h,l,r);}
void SsSeqPlay(short h,char mode,short repeat){event(5,h,mode,repeat);}
static void predict(void)
{
    int active,alert;
    event(1,0x555,0x555,0x555);rr=rg=rb=96;
    active=rm==1 || rm==2;
    alert=(rm==1 && rt==3) || (rm==2 && (rt&1));
    if(alert){event(2,2048,585,585);rr=96;rg=rb=19;event(3,rh,0,0);event(4,rh,126,126);event(5,rh,1,1);}
    if(active)rt=rt-1<0 ? 0 : rt-1;
}
int main(void)
{
    const int modes[]={-1,0,1,2,3,2147483647};
    const int ticks[]={-4096,-3,-1,0,1,2,3,4,5,2147483647};
    const short handles[]={-32768,-1,0,32767};
    int m,t,h,at,k,count=0;
    for(m=0;m<6;m++)for(t=0;t<10;t++)for(h=0;h<4;h++)for(at=-1;at<5;at++)for(k=0;k<3;k++) {
        bad=expected_n=actual_n=0;change_at=at;change_kind=k;
        rm=modes[m];rt=ticks[t];rh=handles[h];rr=17;rg=18;rb=19;
        predicting=1;predict();
        D_800B56EC=modes[m];scene_tick=ticks[t];alert_sequence=handles[h];
        g_fade.rgb[0]=17;g_fade.rgb[1]=18;g_fade.rgb[2]=19;g_fade.rgb[3]=0xA5;g_fade.unk04=-7;g_fade.unk08=0x1234;
        predicting=0;func_800967DC();
        if(bad||actual_n!=expected_n||D_800B56EC!=rm||scene_tick!=rt||alert_sequence!=rh)return 1;
        if(g_fade.rgb[0]!=rr||g_fade.rgb[1]!=rg||g_fade.rgb[2]!=rb||g_fade.rgb[3]!=0xA5||g_fade.unk04!=-7||g_fade.unk08!=0x1234)return 2;
        count++;
    }
    if(count!=4320)return 3;
    printf("%d native scene-alert fixtures passed\n",count);return 0;
}
