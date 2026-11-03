/* Verify message-window state, SDK call order and packed projection outputs. */
#include "../../src/s2d/game/msggradient.c"
extern int printf(const char *, ...);
extern void *memset(void *, int, unsigned long);
extern void *memcpy(void *, const void *, unsigned long);
S2dWin D_800B91EC;
DRAWENV g_s2d_drawenv;
GsOT g_ot_back[2];
int g_draw_side,D_800B0EE4;
short g_s2d_heading;
/* These PS1 labels name fields of the window. Map the native aliases using
   the host struct layout, whose VECTOR contains 64-bit long on this host. */
void define_native_window_aliases(void)
{
    __asm__(".globl D_800B93B8\n.set D_800B93B8,D_800B91EC+%c0\n"
            ".globl D_800B93BC\n.set D_800B93BC,D_800B91EC+%c1\n"
            ".globl D_800B93A4\n.set D_800B93A4,D_800B91EC+%c2\n"
            : : "i"(__builtin_offsetof(S2dWin,x)),"i"(__builtin_offsetof(S2dWin,x0)),
                "i"(__builtin_offsetof(S2dWin,scale)+__builtin_offsetof(VECTOR,vy)));
}
static int bad,used,mutation,events[20],restoring,blue;
static short expected_start_x;
static long saved_x,saved_y,expected_geom_x,expected_geom_y;
static MATRIX *matrix;
static POLY_G4 *polygon;
static short projected[4][2];
static void event(int id){if(used<20)events[used++]=id;else bad=1;}
void S2dWinSetArea(S2dWin *w,int side,int x,int y)
{
    event(1);if(w!=&D_800B91EC||side!=g_draw_side||x!=g_s2d_drawenv.ofs[0]||y!=g_s2d_drawenv.ofs[1]||w->x!=expected_start_x)bad=1;
    if(mutation==1){g_draw_side^=1;g_s2d_heading=!g_s2d_heading;w->flags=0x8000;}
}
void func_8009A368(S2dWin *w,GsOT *ot,u_short pri,int side)
{
    event(2);if(w!=&D_800B91EC||ot!=&g_ot_back[g_draw_side]||side!=g_draw_side||pri)bad=1;
    if(mutation==2){w->flags=0;w->x0=-32760;}
    if(mutation==3){w->flags=0x8000;w->x0=32760;}
}
void SetPolyG4(POLY_G4 *p)
{event(3);polygon=p;memset(p,0,sizeof(*p));p->code=0x38;}
void SetSemiTrans(void *p,int on)
{event(4);if(p!=polygon||on!=1)bad=1;}
void SetShadeTex(void *p,int on)
{
    event(5);if(p!=polygon||on)bad=1;
    if(mutation==4)D_800B91EC.scale.vy=129;
    blue=D_800B91EC.scale.vy==128;
}
void ReadGeomOffset(long *x,long *y)
{
    event(6);*x=saved_x;*y=saved_y;
    if(mutation==5){g_s2d_heading=0;D_800B91EC.x=32760;D_800B91EC.y=-32760;}
    expected_geom_x=(long)D_800B91EC.x+(g_s2d_heading?0:240);expected_geom_y=D_800B91EC.y;
}
void SetGeomOffset(long x,long y)
{
    if(!restoring){event(7);if(x!=expected_geom_x||y!=expected_geom_y)bad=1;restoring=1;}
    else{event(15);if(x!=saved_x||y!=saved_y)bad=1;}
}
MATRIX *RotMatrix(SVECTOR *r,MATRIX *m)
{event(8);matrix=m;memset(m,0,sizeof(*m));if(r!=&D_800B91EC.rot)bad=1;return m;}
MATRIX *TransMatrix(MATRIX *m,VECTOR *v)
{event(9);if(m!=matrix||v!=&D_800B91EC.trans)bad=1;return m;}
MATRIX *ScaleMatrix(MATRIX *m,VECTOR *v)
{event(10);if(m!=matrix||v!=&D_800B91EC.scale)bad=1;return m;}
void SetRotMatrix(MATRIX *m){event(11);if(m!=matrix)bad=1;}
void SetTransMatrix(MATRIX *m){event(12);if(m!=matrix)bad=1;if(mutation==6)D_800B0EE4=-3;}
long RotTransPers4(SVECTOR *a,SVECTOR *b,SVECTOR *c,SVECTOR *d,long *x0,long *x1,long *x2,long *x3,long *depth,long *flag)
{
    int k;SVECTOR *v[4];long *xy[4];event(13);v[0]=a;v[1]=b;v[2]=c;v[3]=d;xy[0]=x0;xy[1]=x1;xy[2]=x2;xy[3]=x3;
    if(b!=a+1||c!=a+2||d!=a+3||depth!=flag)bad=1;
    for(k=0;k<4;k++){
        short x=(short)((k%2)*(D_800B0EE4+1)*16-8),y=(short)((k/2)*24-12);
        if(v[k]->vx!=x||v[k]->vy!=y||v[k]->vz)bad=1;
        projected[k][0]=(short)(x+31);projected[k][1]=(short)(y-17);memcpy(xy[k],projected[k],4);
    }
    *depth=77;return 42;
}
void GsSortPoly(void *vp,GsOT *ot,unsigned short pri)
{
    POLY_G4 *p=vp;event(14);
    if(p!=polygon||ot!=&g_ot_back[g_draw_side]||pri||p->code!=0x38)bad=1;
    if(p->r0||p->g0||p->b0!=(blue?255:0)||p->r1||p->g1||p->b1!=(blue?255:0)||p->r2!=(blue?0:96)||p->g2!=(blue?0:64)||p->b2!=255||p->r3!=(blue?0:96)||p->g3!=(blue?0:64)||p->b3!=255)bad=1;
    if(p->x0!=projected[0][0]||p->y0!=projected[0][1]||p->x1!=projected[1][0]||p->y1!=projected[1][1]||p->x2!=projected[2][0]||p->y2!=projected[2][1]||p->x3!=projected[3][0]||p->y3!=projected[3][1])bad=1;
    g_draw_side^=1;
}
int main(void)
{
    const u_short flags[]={0,1,0x8000,0x8001,0xFFFF};const short positions[]={-32768,-32760,-240,-1,0,1,32550,32767};
    const int scales[]={0,128,129},widths[]={-1,0,1,10,123,2047};
    int fi,heading,si,side,xi,wi,m,k,n;unsigned long cases=0;
    {void *volatile a=&D_800B93B8,*volatile b=&D_800B91EC.x;
    if(a!=b){printf("FAIL native x alias\n");return 1;}
    a=&D_800B93BC;b=&D_800B91EC.x0;if(a!=b){printf("FAIL native x0 alias\n");return 1;}
    a=&D_800B93A4;b=&D_800B91EC.scale.vy;if(a!=b){printf("FAIL native scale alias\n");return 1;}}
    for(fi=0;fi<5;fi++)for(heading=-1;heading<=1;heading++)for(si=0;si<3;si++)for(side=0;side<2;side++)
    for(xi=0;xi<8;xi++)for(wi=0;wi<6;wi++)for(m=0;m<7;m++){
        int expected[20];u_short final_flags=flags[fi];short final_x;
        memset(&D_800B91EC,0,sizeof(D_800B91EC));D_800B91EC.flags=flags[fi];D_800B91EC.x=positions[xi];D_800B91EC.y=positions[7-xi];D_800B91EC.x0=(short)(positions[xi]^1234);D_800B91EC.scale.vy=scales[si];
        g_s2d_drawenv.ofs[0]=-256;g_s2d_drawenv.ofs[1]=120;g_draw_side=side;g_s2d_heading=heading;D_800B0EE4=widths[wi];
        expected_start_x=(short)(positions[xi]+(heading?0:240));final_x=expected_start_x;
        if(final_flags){if(m==1||m==3)final_flags=0x8000;if(m==2)final_flags=0;final_x=m==2?-32760:m==3?32760:D_800B91EC.x0;}
        if((final_flags&0x8000)&&m==5)final_x=32760;
        n=0;if(flags[fi]){expected[n++]=1;expected[n++]=2;}if(final_flags&0x8000)for(k=3;k<=15;k++)expected[n++]=k;
        mutation=m;bad=used=restoring=0;saved_x=100000+xi;saved_y=-200000-wi;
        func_800937FC();
        if(used!=n||D_800B91EC.x!=final_x)bad=1;for(k=0;k<n;k++)if(events[k]!=expected[k])bad=1;
        if(bad){printf("FAIL %d %d %d %d %d %d %d used %d expected %d x %d/%d\n",fi,heading,si,side,xi,wi,m,used,n,D_800B91EC.x,final_x);return 1;}cases++;
    }
    printf("PASS %lu message-gradient fixtures\n",cases);return 0;
}
