/* Execute production C; replace only the PS1 linker alias with a host bank. */
#define __asm__(symbol)
#include "../../src/casino/game/gridselect.c"
#undef __asm__
extern int printf(const char *, ...);
u_char g_grid_select_palette_on, D_80099B10[512], D_800A8700[16], g_casino_step;
short D_800A6748[3][298];
CasinoLayout *D_800A2BC8[852];
CasinoLayoutDef grid_choices[2];
int g_casino_timer;
static CasinoLayout layouts[3];
static CasinoObj object;
typedef struct { int kind,a,b,c; } Event;
static Event expected[16];
static int predicting, expected_n, actual_n, bad, change_at, change_kind;
static int rt,rp,rs,rstep;
static void event(int kind,int a,int b,int c)
{
    Event e={kind,a,b,c}; int n;
    if(predicting) {n=expected_n++;if(n>=16){bad=1;return;}expected[n]=e;}
    else {n=actual_n++;if(n>=expected_n || expected[n].kind!=kind || expected[n].a!=a || expected[n].b!=b || expected[n].c!=c)bad=1;}
    if(n!=change_at)return;
    if(predicting) {
        if(change_kind==0)rt=kind==1 ? 0 : kind==3 ? 32 : 44;
        else {rp=(rp+1)%9;rs=(rs+1)%3;}
    } else {
        if(change_kind==0)g_casino_timer=kind==1 ? 0 : kind==3 ? 32 : 44;
        else {D_800A8700[1]=(D_800A8700[1]+1)%9;D_800A8700[2]=(D_800A8700[2]+1)%3;}
    }
}
static int panel(CasinoLayout*p) {int i;for(i=0;i<3;i++)if(p==&layouts[i])return i;bad=1;return -1;}
void CasinoStartPalAnim(CasinoPalAnim*p) {if((void*)p!=(void*)(D_80099B10+360))bad=1;event(1,0,0,0);}
void func_8008A9DC(int a,short b,short c) {event(2,a,b,c);}
void func_8008C5DC(CasinoLayout*p,short*codes) {
    int row,col,found=0;
    for(row=0;row<3;row++)for(col=0;col<9;col++)if(codes==&D_800A6748[row][1+col*33]) {event(3,panel(p),row,col);found=1;}
    if(!found)bad=1;
}
void CasinoTween(CasinoLayout*p,int dx,short dy,short dw,short dh,short frames) {
    if(dx==-160) {if(dy||dw||dh||frames!=32)bad=1;event(4,panel(p),dx,frames);}
    else {if(dx!=-24||dy!=-24||dw!=48||dh!=48||frames!=12)bad=1;event(6,panel(p),dx,frames);}
}
void CasinoShowLayout(CasinoLayout*p,u_char mode) {if(mode)bad=1;event(5,panel(p),0,0);}
void CasinoStartAnim(CasinoObj*p,short frames,short rx,short ry,short rz,long tx,long ty,long tz,long sx,long sy,long sz) {
    if(p!=&object||frames!=16||rx!=2048||ry||rz||tx||ty||tz||sx||sy||sz)bad=1;event(7,0,0,0);
}
void func_8008B428(void) {event(8,0,0,0);}
void func_8008AA64(int player,int active) {event(9,player,active,0);}
static void predict(int palette)
{
    if(!palette)event(1,0,0,0);
    if(rt==0) {event(2,1,-128,32);event(3,rp%2,rs,rp);event(4,rp%2,-160,32);}
    if(rt==32) {event(5,1-rp%2,0,0);event(6,2,-24,12);event(7,0,0,0);}
    if(rt>=44) {event(8,0,0,0);event(9,rp,1,0);rstep=112;rt=-1;}
}
int main(void)
{
    const int timers[]={-2147483647-1,-1,0,1,31,32,33,43,44,2147483647};
    int t,flag,p,s,at,kind,i,j,count=0;
    grid_choices[0].l=&layouts[0];grid_choices[1].l=&layouts[1];
    D_800A2BC8[56]=&layouts[2];D_800A2BC8[851]=(CasinoLayout*)&object;
    for(i=0;i<3;i++)for(j=0;j<298;j++)D_800A6748[i][j]=(short)(i*1000+j);
    for(t=0;t<10;t++)for(flag=0;flag<2;flag++)for(p=0;p<9;p++)for(s=0;s<3;s++)
        for(at=-1;at<9;at++)for(kind=0;kind<2;kind++) {
            change_at=at;change_kind=kind;bad=actual_n=expected_n=0;
            rt=timers[t];rp=p;rs=s;rstep=77;predicting=1;predict(flag);
            g_casino_timer=timers[t];g_casino_step=77;g_grid_select_palette_on=flag;
            for(i=0;i<16;i++)D_800A8700[i]=0xA5;
            D_800A8700[1]=p;D_800A8700[2]=s;predicting=0;func_80088C8C();
            if(bad||actual_n!=expected_n||g_casino_timer!=rt||g_casino_step!=rstep||D_800A8700[1]!=rp||D_800A8700[2]!=rs)return 1;
            for(i=0;i<16;i++)if(i!=1&&i!=2&&D_800A8700[i]!=0xA5)return 2;
            for(i=0;i<3;i++)for(j=0;j<298;j++)if(D_800A6748[i][j]!=(short)(i*1000+j))return 3;
            for(i=0;i<852;i++)if(D_800A2BC8[i]!=(i==56?&layouts[2]:i==851?(CasinoLayout*)&object:0))return 4;
            count++;
        }
    if(count!=10800)return 5;
    printf("%d native grid-selection fixtures passed\n",count);return 0;
}
