#include <persona/casino/casino.h>
extern int puts(const char *);
extern void func_80087F48(u_char);
CasinoLayoutDef D_800A1764,D_800A17B8,D_800A1620;
CasinoObj D_800A16F8;
static CasinoLayout panels[4];
static int n,bad,distance,mutate;
void CasinoTween(CasinoLayout *l,int dx,short dy,short dw,short dh,short frames) {
    CasinoLayout *expected=n==0?&panels[0]:n==1?&panels[mutate?3:1]:&panels[2];
    if(n>=3 || l!=expected || dx!=(n==1?distance:-distance) || dy || dw || dh || frames!=32) bad=1;
    n++;if(mutate && n==1) D_800A17B8.l=&panels[3];
}
void CasinoStartAnim(CasinoObj *o,short frames,short rx,short ry,short rz,long x,long y,long z,long sx,long sy,long sz) {
    if(n!=3 || o!=&D_800A16F8 || frames!=32 || rx || ry || rz || x!=distance || y || z || sx || sy || sz) bad=1;
    n++;
}
int main(void) {
    int mode,change;
    for(mode=0;mode<256;mode++) for(change=0;change<2;change++) {
        D_800A1764.l=&panels[0];D_800A17B8.l=&panels[1];D_800A1620.l=&panels[2];
        n=bad=0;mutate=change;distance=mode?320:-320;
        func_80087F48(mode);if(bad || n!=4) return 1;
    }
    puts("512 native panel-slide fixtures passed");return 0;
}
