#include <persona/casino/blackjack.h>
extern int puts(const char *);
u_char D_800A8678[2], g_bj_count[2], g_bj_natural[2];
static struct { int first, count, r, g, b, frames; } calls[6];
static int n, mutate;
void CasinoFade(int first, int count, u_char r, u_char g, u_char b, short frames)
{
    if(n>=6) {n++;return;}
    calls[n].first=first;calls[n].count=count;
    calls[n].r=r;calls[n].g=g;calls[n].b=b;calls[n].frames=frames;
    n++;
    if(mutate && n==2) {g_bj_count[0]=4;g_bj_count[1]=3;}
    if(mutate && first==727) g_bj_natural[1]=2;
}
static int row(int i,int first,int count,int brightness) {
    return calls[i].first==first && calls[i].count==count &&
        calls[i].r==brightness && calls[i].g==brightness &&
        calls[i].b==brightness && calls[i].frames==8;
}
static int check(unsigned char mode,int first_count,int second_count,int natural,int change) {
    int brightness=mode==1?128:mode==2?64:0;
    int i=0;
    g_bj_count[0]=first_count;g_bj_count[1]=second_count;
    g_bj_natural[0]=19;g_bj_natural[1]=natural;
    D_800A8678[0]=29;D_800A8678[1]=37;n=0;mutate=change;
    func_8007F668(mode);
    if(D_800A8678[0]!=29 || D_800A8678[1]!=mode || g_bj_natural[0]!=19) return 0;
    if(!row(i++,580,20,brightness) || !row(i++,520,20,brightness)) return 0;
    if(change) {first_count=4;second_count=3;natural=2;}
    if(second_count>=3 && !row(i++,620-(first_count+second_count+1)*20,(second_count-2)*20,brightness)) return 0;
    if(!row(i++,694,3,brightness) || !row(i++,727,10,brightness)) return 0;
    if(natural==2) {if(!row(i++,907,4,brightness)) return 0;}
    else if(natural!=1) {if(!row(i++,891,8,brightness)) return 0;}
    return n==i;
}
int main(void) {
    int mode;
    for(mode=0;mode<256;mode++) {
        if(!check(mode,2,2,1,0)) return 1;
        if(!check(mode,2,3,2,0)) return 2;
        if(!check(mode,255,255,8,0)) return 3;
        if(!check(mode,0,0,0,1)) return 4;
    }
    puts("1024 native second-hand fade fixtures passed");return 0;
}
