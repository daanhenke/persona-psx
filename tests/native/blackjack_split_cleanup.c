/* Host-side transition and callback-order checks; no PS1 hardware is emulated. */
#include "../../src/casino/game/bjsplitcleanup.c"
extern int printf(const char *, ...);
extern void *memset(void *, int, unsigned long);
extern int memcmp(const void *, const void *, unsigned long);
CasinoPalAnim D_80099990[1];
CasinoLayoutDef D_8009ACE4[2], D_8009AE2C[2], D_8009AED0[2], D_8009B91C[1], D_8009B2B4[4], D_8009B6D8[1];
CasinoLayout *D_8009B928, *D_8009B930;
CasinoObj D_8009C294[2], D_8009C2B4[2], D_8009C42C, D_8009C43C, D_8009C568, D_8009C578, D_8009B7B0[1], g_casino_objs[25];
unsigned char D_800A8768[2], D_800A8818[2], g_bj_result_palette_on, g_bj_card_obj, g_bj_count[2], g_bj_label_n[3], g_casino_bet, g_casino_step;
short g_casino_seqs[16], g_casino_main_vab;
int g_casino_timer;
static CasinoLayout layouts[13];
static CasinoXform transforms[8];
static CasinoObj *result_objects[8] = {D_8009C294,D_8009C294+1,D_8009C2B4,D_8009C2B4+1,&D_8009C42C,&D_8009C43C,&D_8009C568,&D_8009C578};
enum {PAL,SEQ,ANIM,SPRITE,LABEL,TWEEN,BET,CLUT,BUILD};
typedef struct {int kind,id,a,b,c,d,e;} Event;
static Event actual[100], expected[100];
static int na,ne,bad,mutation;
static void add(Event *dst,int *n,int k,int id,int a,int b,int c,int d,int e)
{Event x={k,id,a,b,c,d,e};if(*n>=100){bad=1;return;}dst[(*n)++]=x;}
#define EXPECT(k,id,a,b,c,d,e) add(expected,&ne,k,id,a,b,c,d,e)
static void emit(int k,int id,int a,int b,int c,int d,int e)
{
    add(actual,&na,k,id,a,b,c,d,e);
    if(mutation==1&&k==TWEEN&&id==0&&a==0)g_casino_timer=32;
    if(mutation==2&&k==BUILD)g_casino_timer=33;
    if(mutation==3&&k==PAL)g_casino_timer=0;
}
void CasinoStartPalAnim(CasinoPalAnim *p){if(p!=D_80099990)bad=1;emit(PAL,0,0,0,0,0,0);}
void CasinoPlaySeq(short *seq,u_long *addr,short vab)
{if(seq!=g_casino_seqs||addr!=(u_long*)0x1387D0||vab!=-123)bad=1;emit(SEQ,0,0,0,0,0,0);}
void CasinoStartAnim(CasinoObj *p,short frames,short rx,short ry,short rz,long x,long y,long z,long sx,long sy,long sz)
{
    int i,id=-1;for(i=0;i<25;i++)if(p==g_casino_objs+i)id=i;
    for(i=0;i<8;i++)if(p==result_objects[i])id=100+i;
    if(p==D_8009B7B0)id=200;
    if(id<0||frames!=32||rx||ry||rz||x!=-320||y||z||sx||sy||sz)bad=1;
    emit(ANIM,id,0,0,0,0,0);
}
void CasinoSpritesSetOn(short first,short count,int on){emit(SPRITE,first,count,on,0,0,0);}
void func_8007EE78(void *p,int n,int frames)
{int id=p==D_8009ACE4?0:p==D_8009AE2C?1:p==D_8009AED0?2:-1;if(id<0||frames!=8)bad=1;emit(LABEL,id,n,0,0,0,0);}
void CasinoTween(CasinoLayout *p,int x,int y,int w,int h,int frames)
{int i,id=-1;for(i=0;i<13;i++)if(p==layouts+i)id=i;if(id<0)bad=1;emit(TWEEN,id,x,y,w,h,frames);}
void func_8007E614(unsigned char bet){emit(BET,bet,0,0,0,0,0);}
void CasinoQueueCluts(int first,int count,int color){emit(CLUT,first,count,color,0,0,0);}
void CasinoBuildLayouts(CasinoLayoutDef *p,int count)
{if(p!=D_8009B91C||count!=1)bad=1;emit(BUILD,0,0,0,0,0,0);}
static void expected_trace(int timer,int palette,int c0,int c1,int obj,int r0,int r1,int o0,int o1,int *final_timer)
{
    int h,i,results[2],options[2];results[0]=r0;results[1]=r1;options[0]=o0;options[1]=o1;
    if(!palette){EXPECT(PAL,0,0,0,0,0,0);if(mutation==3)timer=0;}
    if(timer==0){
        EXPECT(SEQ,0,0,0,0,0,0);
        for(i=24;i>obj;i--)EXPECT(ANIM,i,0,0,0,0,0);
        for(h=0;h<2;h++)if((h?c1:c0)>=3)for(i=0;i<4;i++)EXPECT(SPRITE,60+7*h+10*i,3,1,0,0,0);
        for(h=0;h<3;h++){EXPECT(LABEL,h,-g_bj_label_n[h],0,0,0,0);EXPECT(TWEEN,6+(h?h+1:0),8,0,-16,0,2);}
        EXPECT(BET,0,0,0,0,0,0);EXPECT(CLUT,748,7,0x7D64,0,0,0);EXPECT(CLUT,755,7,0x7D64,0,0,0);
        for(h=0;h<2;h++){
            if(results[h]==11){EXPECT(ANIM,100+2*h,0,0,0,0,0);EXPECT(ANIM,101+2*h,0,0,0,0,0);}
            if(results[h]==12)EXPECT(ANIM,104+h,0,0,0,0,0);
            if(results[h]==13)EXPECT(ANIM,106+h,0,0,0,0,0);
        }
        for(h=0;h<2;h++)if(options[h]==1)EXPECT(TWEEN,10+h,-320,0,0,0,32);
    }
    if(timer==8){for(i=0;i<6;i++)EXPECT(TWEEN,i,0,8,0,-16,8);if(mutation==1)timer=32;}
    if(timer==32){
        for(h=0;h<2;h++)if(options[h]==1)EXPECT(SPRITE,873+h,1,0,0,0,0);
        EXPECT(BUILD,0,0,0,0,0,0);if(mutation==2)timer=33;
    }
    if(timer>=33){
        EXPECT(SPRITE,694,3,0,0,0,0);EXPECT(SPRITE,691,3,0,0,0,0);
        EXPECT(SPRITE,717,10,0,0,0,0);EXPECT(SPRITE,727,10,0,0,0,0);
        EXPECT(TWEEN,12,320,0,0,0,32);EXPECT(ANIM,200,0,0,0,0,0);
    }
    *final_timer=timer;
}
int main(void)
{
    static const int timers[]={-1,0,7,8,9,31,32,33,100};
    static const int results[]={0,10,11,12,13,14,255},cards[]={0,12,24,255},counts[]={2,3,255};
    int i,t,r0,r1,o,p,c,obj,mode,h,final_timer,expected_x[8],expected_rot[8];unsigned cases=0;
    for(i=0;i<8;i++)result_objects[i]->xform=transforms+i;
    for(i=0;i<2;i++){D_8009ACE4[i].l=layouts+i;D_8009AE2C[i].l=layouts+2+i;D_8009AED0[i].l=layouts+4+i;}
    for(i=0;i<4;i++)D_8009B2B4[i].l=layouts+6+i;
    D_8009B928=layouts+10;D_8009B930=layouts+11;D_8009B6D8[0].l=layouts+12;g_casino_main_vab=-123;
    for(mode=0;mode<4;mode++)for(t=0;t<9;t++)for(r0=0;r0<7;r0++)for(r1=0;r1<7;r1++)for(o=0;o<9;o++)for(p=0;p<2;p++)for(c=0;c<3;c++)for(obj=0;obj<4;obj++){
        mutation=mode;na=ne=0;g_casino_timer=timers[t];g_bj_result_palette_on=p;
        D_800A8768[0]=results[r0];D_800A8768[1]=results[r1];D_800A8818[0]=o%3;D_800A8818[1]=o/3;
        g_bj_count[0]=counts[c];g_bj_count[1]=counts[(c+1)%3];g_bj_card_obj=cards[obj];g_casino_bet=217;g_casino_step=99;
        g_bj_label_n[0]=0;g_bj_label_n[1]=128;g_bj_label_n[2]=255;
        for(i=0;i<8;i++){memset(transforms+i,0,sizeof transforms[i]);transforms[i].trans.vx=expected_x[i]=987;transforms[i].rot.vy=expected_rot[i]=-123;}
        expected_trace(timers[t],p,g_bj_count[0],g_bj_count[1],cards[obj],results[r0],results[r1],o%3,o/3,&final_timer);
        if(final_timer>=33){
            for(h=0;h<2;h++){
                int r=results[h?r1:r0];
                if(r==11){expected_x[h*2]=h?400:240;expected_x[h*2+1]=h?-240:-400;}
                if(r==12||r==13){i=4+(r-12)*2+h;expected_x[i]=h?80:-80;expected_rot[i]=2048;}
            }
        }
        func_8007CBE4();
        if(na!=ne||memcmp(actual,expected,(unsigned long)na*sizeof actual[0]))bad=1;
        if(g_casino_timer!=(final_timer>=33?-1:final_timer)||g_casino_step!=(final_timer>=33?18:99))bad=1;
        if(g_casino_bet!=((timers[t]==0||(!p&&mode==3))?0:217))bad=1;
        if(final_timer==32||(mode==2&&timers[t]==32)||(mode==1&&timers[t]==8)){
            if(D_800A8818[0]||D_800A8818[1])bad=1;
        }else if(D_800A8818[0]!=o%3||D_800A8818[1]!=o/3)bad=1;
        for(i=0;i<8;i++)if(transforms[i].trans.vx!=expected_x[i]||transforms[i].rot.vy!=expected_rot[i])bad=1;
        if(bad){printf("FAIL mode=%d timer=%d results=%d/%d options=%d palette=%d count=%d obj=%d events=%d/%d\n",mode,timers[t],results[r0],results[r1],o,p,c,cards[obj],na,ne);return 1;}
        cases++;
    }
    printf("PASS %u split-cleanup transition fixtures\n",cases);return 0;
}
