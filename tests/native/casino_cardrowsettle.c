#define __asm__(symbol)
#include "../../src/casino/game/cardrowsettle.c"
#undef __asm__
extern int printf(const char *, ...);
s8 row_work[512];
CasinoLayout *D_800A24B0[21][2];
u_char *D_800B3E28[21];
static CasinoLayout layouts[21];
static short textures[21][3];
static u_char maps[21][256];
static s8 before[512];
static int row_id, mode, events, bad, eligible, done, seed;
static s8 expected_timer;
static void timer_change(void) {
    if(mode==1) {row_work[row_id+5]=126;expected_timer=126;}
}
void func_80087124(s8 row,u_char *symbols,s8 *positions,s8 *previous)
{
    int i;
    if(events || row!=row_id || symbols!=(u_char*)row_work+411 || positions!=row_work+348 || previous!=row_work+390)bad=1;
    events++;timer_change();
    if(mode==2)for(i=0;i<4;i++)row_work[395+row_id*4+i]=(s8)(seed+90+i);
}
void CasinoSpritesSetOn(short first,short count,int on)
{
    if(events==1) {if(first!=row_id*4+500 || count!=4 || on!=1)bad=1;}
    else if(events==2) {if(first!=row_id*8+461 || count!=8 || on!=0)bad=1;}
    else bad=1;
    events++;timer_change();
}
void CasinoTexLayout(CasinoLayout *layout)
{
    int idx=row_id*4+5+done, digit=row_work[395+row_id*4+done];
    if(events!=3+done || done>=4 || layout!=&layouts[idx])bad=1;
    if(textures[idx][1]!=D_800B3E28[idx][digit])bad=1;
    events++;done++;timer_change();
    if(mode==3 && done<4) {
        row_work[395+row_id*4+done]=(s8)(seed-70+done);
        D_800B3E28[idx+1]=maps[20-(idx+1)]+128;
    }
    if(mode==4)row_work[row_id+1]=99;
}
static int check(int step,int timer,int value)
{
    int i,j;seed=value;events=bad=done=0;
    for(i=0;i<512;i++)row_work[i]=17;
    row_work[row_id+1]=step;row_work[row_id+5]=timer;
    for(i=0;i<4;i++)row_work[395+row_id*4+i]=(s8)(seed+i);
    for(i=0;i<512;i++)before[i]=row_work[i];
    expected_timer=timer;eligible=(step==5 && (s8)timer>=17);
    for(i=0;i<21;i++) {
        for(j=0;j<256;j++)maps[i][j]=(u_char)(i*19+j*3);
        D_800B3E28[i]=maps[i]+128;
        D_800A24B0[i][0]=&layouts[i];D_800A24B0[i][1]=0;
        layouts[i].tex_idx=&textures[i][1];
        textures[i][0]=1234;textures[i][1]=-1;textures[i][2]=5678;
    }
    func_80085B74(row_id);
    if(bad || events!=(eligible?7:0) || done!=(eligible?4:0) || row_work[row_id+1]!=(eligible?6:step) || row_work[row_id+5]!=(s8)(expected_timer+1))return 1;
    for(i=0;i<21;i++) {
        if(textures[i][0]!=1234 || textures[i][2]!=5678 || layouts[i].tex_idx!=&textures[i][1] || D_800A24B0[i][0]!=&layouts[i] || D_800A24B0[i][1])return 2;
        if(!(eligible && i>=row_id*4+5 && i<row_id*4+9) && textures[i][1]!=-1)return 3;
    }
    for(i=0;i<512;i++) {
        if(i==row_id+1 || i==row_id+5)continue;
        if(eligible && ((mode==2 && i>=395+row_id*4 && i<399+row_id*4) || (mode==3 && i>395+row_id*4 && i<399+row_id*4)))continue;
        if(row_work[i]!=before[i])return 4;
    }
    return 0;
}
int main(void)
{
    const int steps[]={0,4,5,6,99};const int timers[]={-128,-1,0,15,16,17,18,127};
    int s,t,v,count=0;
    for(row_id=0;row_id<4;row_id++)for(mode=0;mode<5;mode++) {
        for(s=0;s<5;s++)for(t=0;t<8;t++) {if(check(steps[s],timers[t],31))return 1;count++;}
        for(v=0;v<256;v++) {if(check(5,17,v))return 2;count++;}
    }
    if(count!=5920)return 3;
    printf("%d native card-row fixtures passed\n",count);return 0;
}
