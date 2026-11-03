/* Native callback/geometry checks; GPU projection itself is stubbed. */
#include "../../src/s2d/game/panelgradient.c"
extern int printf(const char *, ...);
extern void *memset(void *, int, unsigned long);
extern void *memcpy(void *, const void *, unsigned long);
S2dGradientPanel D_800B5E28[3];
VECTOR D_8006509C;
static VECTOR copied;
static GsOT tables[2], *expected_ot;
static MATRIX *current_matrix;
static POLY_G4 *current_poly;
static int bad, stage, row, mutation, priority, ox, oy;
static short expected_xy[4][2];
static u_char red;
static void check_stage(int want) { if (stage++ != want) bad=1; }
void SetPolyG4(POLY_G4 *p)
{
    check_stage(0);current_poly=p;memset(p,0,sizeof(*p));p->code=0x38;
}
void SetSemiTrans(void *p,int enabled)
{check_stage(1);if(p!=current_poly||enabled!=1)bad=1;}
void SetShadeTex(void *p,int enabled)
{check_stage(2);if(p!=current_poly||enabled)bad=1;}
static int half_floor(int x) {return x>=0?x/2:(x-1)/2;}
void SetGeomOffset(long x,long y)
{
    int k,mode;S2dGradientPanel *p;
    row++;check_stage(3+row*8);p=&D_800B5E28[row];
    if(x!=p->x||y!=p->y)bad=1;
    if(mutation==1){p->flags=(p->flags&~6)|((row%3)<<1);p->w=(short)(p->w+13);p->h=(short)(p->h-7);}
    if(mutation==2){D_8006509C.vx=98765;D_8006509C.vy=-65432;D_8006509C.vz=12345;D_8006509C.pad=76;}
    mode=(p->flags>>1)&3;
    if(mode==0){ox=0;oy=0;}
    if(mode==1){ox=-half_floor(p->w);oy=-half_floor(p->h);}
    if(mode==2){ox=-p->w;oy=0;}
    for(k=0;k<4;k++){expected_xy[k][0]=(short)(ox+(k%2)*p->w);expected_xy[k][1]=(short)(oy+(k/2)*p->h);}
}
MATRIX *RotMatrix(SVECTOR *r,MATRIX *m)
{
    check_stage(4+row*8);if(r!=&D_800B5E28[row].rotation)bad=1;
    current_matrix=m;memset(m,0,sizeof(*m));m->m[0][0]=123;
    if(mutation==3){D_800B5E28[row].w=1234;D_800B5E28[row].h=-4567;D_800B5E28[row].scale.vx+=19;}
    return m;
}
MATRIX *TransMatrix(MATRIX *m,VECTOR *v)
{
    check_stage(5+row*8);if(m!=current_matrix||v->vx!=copied.vx||v->vy!=copied.vy||v->vz!=copied.vz||v->pad!=copied.pad)bad=1;
    m->t[0]=v->vx;m->t[1]=v->vy;m->t[2]=v->vz;return m;
}
MATRIX *ScaleMatrix(MATRIX *m,VECTOR *v)
{
    check_stage(6+row*8);if(m!=current_matrix||v!=&D_800B5E28[row].scale)bad=1;
    if(mutation==4 && row<2){D_800B5E28[row+1].flags=2;D_800B5E28[row+1].w=-511;D_800B5E28[row+1].h=513;}
    return m;
}
void SetRotMatrix(MATRIX *m)
{check_stage(7+row*8);if(m!=current_matrix||m->m[0][0]!=123)bad=1;}
void SetTransMatrix(MATRIX *m)
{check_stage(8+row*8);if(m!=current_matrix||m->t[0]!=copied.vx||m->t[1]!=copied.vy||m->t[2]!=copied.vz)bad=1;}
long RotTransPers4(SVECTOR *a,SVECTOR *b,SVECTOR *c,SVECTOR *d,long *x0,long *x1,long *x2,long *x3,long *depth,long *flag)
{
    int k;SVECTOR *v[4];long *xy[4];short pair[2];
    check_stage(9+row*8);v[0]=a;v[1]=b;v[2]=c;v[3]=d;xy[0]=x0;xy[1]=x1;xy[2]=x2;xy[3]=x3;
    if(b!=a+1||c!=a+2||d!=a+3||depth!=flag)bad=1;
    for(k=0;k<4;k++){
        if(v[k]->vx!=expected_xy[k][0]||v[k]->vy!=expected_xy[k][1]||v[k]->vz)bad=1;
        pair[0]=(short)(expected_xy[k][0]+17);pair[1]=(short)(expected_xy[k][1]-23);
        /* The PS1 helper writes two packed 16-bit coordinates, not a host long. */
        memcpy(xy[k],pair,sizeof(pair));
    }
    *depth=32767;return -123;
}
void GsSortPoly(void *vp,GsOT *ot,unsigned short pri)
{
    POLY_G4 *p=vp;int k;short xy[4][2];
    check_stage(10+row*8);
    if(p!=current_poly||ot!=expected_ot||pri!=(unsigned short)priority)bad=1;
    if(p->code!=0x38||p->r0!=red||p->g0||p->b0||p->r1||p->g1||p->b1||p->r2!=96||p->g2!=64||p->b2!=255||p->r3!=96||p->g3!=64||p->b3!=255)bad=1;
    xy[0][0]=p->x0;xy[0][1]=p->y0;xy[1][0]=p->x1;xy[1][1]=p->y1;xy[2][0]=p->x2;xy[2][1]=p->y2;xy[3][0]=p->x3;xy[3][1]=p->y3;
    for(k=0;k<4;k++)if(xy[k][0]!=(short)(expected_xy[k][0]+17)||xy[k][1]!=(short)(expected_xy[k][1]-23))bad=1;
    if(mutation==5){p->r0=red=(u_char)(33+row);}
}
int main(void)
{
    const short sizes[]={-32768,-513,-1,0,1,511,32767};
    const int priorities[]={-1,0,1,65535,65536};
    int first,second,third,w,h,m,pi,k;unsigned long cases=0;
    for(first=0;first<3;first++)for(second=0;second<4;second++)for(third=0;third<4;third++)
    for(w=0;w<7;w++)for(h=0;h<7;h++)for(m=0;m<6;m++)for(pi=0;pi<5;pi++){
        int modes[3];modes[0]=first;modes[1]=second;modes[2]=third;
        memset(D_800B5E28,0xA5,sizeof(D_800B5E28));
        for(k=0;k<3;k++){
            S2dGradientPanel *p=&D_800B5E28[k];p->flags=(u_short)(0xFFF9|(modes[k]<<1));
            p->x=(short)(sizes[w]+k);p->y=(short)(sizes[h]-k);p->w=sizes[(w+k)%7];p->h=sizes[(h+k)%7];
            p->scale.vx=100+k;p->scale.vy=200+k;p->scale.vz=300+k;p->scale.pad=400+k;
        }
        copied.vx=-123456;copied.vy=654321;copied.vz=4096;copied.pad=87;D_8006509C=copied;
        mutation=m;priority=priorities[pi];expected_ot=&tables[(w+h)&1];bad=stage=red=0;row=-1;ox=oy=0;
        func_80099AD8(expected_ot,priority);
        if(bad||stage!=27||row!=2){printf("FAIL %d %d %d %d %d %d %d stage %d row %d\n",first,second,third,w,h,m,pi,stage,row);return 1;}
        cases++;
    }
    printf("PASS %lu panel-gradient fixtures\n",cases);return 0;
}
