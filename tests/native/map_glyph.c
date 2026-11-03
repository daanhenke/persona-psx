#include "../../src/s2d/game/mapglyph.c"
extern int printf(const char *,...);
extern void *memset(void *,int,unsigned long);
extern void *memcpy(void *,const void *,unsigned long);
u_short D_800A85D0;
static GsOT tables[2],*expected_ot;
static POLY_FT4 *packet;
static u_char text_buffer[258];
static short projected[4][2];
static int bad,stage,row,mutation,x,y,color;
static u_short page,clut;
static void step(int expected){if(stage++!=expected)bad=1;}
void SetPolyFT4(POLY_FT4 *p)
{step(0);packet=p;memset(p,0,sizeof(*p));p->code=0x2C;p->r0=13;p->g0=27;p->b0=41;}
void SetSemiTrans(void *p,int on)
{step(1);if(p!=packet||on)bad=1;}
void SetShadeTex(void *p,int on)
{step(2);if(p!=packet||on!=1)bad=1;if(mutation==1)D_800A85D0+=37;}
u_short GetTPage(int tp,int abr,int tx,int ty)
{step(3);if(tp||abr||tx!=D_800A85D0+320||ty!=384)bad=1;return page=(u_short)(tx^0x73C1);}
long RotTransPers4(SVECTOR *a,SVECTOR *b,SVECTOR *c,SVECTOR *d,long *x0,long *x1,long *x2,long *x3,long *depth,long *flag)
{
    SVECTOR *v[4];long *xy[4];int k;
    step(4+row*3);v[0]=a;v[1]=b;v[2]=c;v[3]=d;xy[0]=x0;xy[1]=x1;xy[2]=x2;xy[3]=x3;
    if(b!=a+1||c!=a+2||d!=a+3||depth!=flag)bad=1;
    for(k=0;k<4;k++){
        short ex=(short)(x+(k%2)*7+row*8),ey=(short)(y+(k/2)*12);
        if(v[k]->vx!=ex||v[k]->vy!=ey||v[k]->vz)bad=1;
        projected[k][0]=(short)(ex+21);projected[k][1]=(short)(ey-19);
        memcpy(xy[k],projected[k],sizeof projected[k]);
    }
    *depth=123456;
    if(mutation==2)text_buffer[row]=(u_char)((text_buffer[row]+17)%255);
    return -17;
}
u_short GetClut(int cx,int cy)
{
    step(5+row*3);if(cx!=512||cy!=color+247)bad=1;
    if(mutation==3 && text_buffer[row+1]!=255)text_buffer[row+1]=(u_char)((text_buffer[row+1]+33)%255);
    return clut=(u_short)(cy^0xAC13);
}
void GsSortPoly(void *p,GsOT *ot,unsigned short pri)
{
    POLY_FT4 *q=p;unsigned ch=text_buffer[row],u=(ch%31)*8,v=(ch/31)*12;
    short actual[4][2];int k;
    step(6+row*3);
    if(q!=packet||ot!=expected_ot||pri||q->code!=0x2C||q->tpage!=page||q->clut!=clut)bad=1;
    if(q->r0!=13||q->g0!=27||q->b0!=41)bad=1;
    if(q->u0!=(u_char)u||q->u1!=(u_char)(u+8)||q->u2!=(u_char)u||q->u3!=(u_char)(u+8))bad=1;
    if(q->v0!=(u_char)(v+128)||q->v1!=(u_char)(v+128)||q->v2!=(u_char)(v+140)||q->v3!=(u_char)(v+140))bad=1;
    actual[0][0]=q->x0;actual[0][1]=q->y0;actual[1][0]=q->x1;actual[1][1]=q->y1;
    actual[2][0]=q->x2;actual[2][1]=q->y2;actual[3][0]=q->x3;actual[3][1]=q->y3;
    for(k=0;k<4;k++)if(actual[k][0]!=projected[k][0]||actual[k][1]!=projected[k][1])bad=1;
    row++;
}
int main(void)
{
    const int coords[]={-32768,-257,-1,0,32760};
    const int colors[]={-32768,-248,-247,0,32767};
    const u_short pages[]={0,512,65535};
    unsigned code,ci,co,pg,m,len,k,fixtures=0;
    for(code=0;code<=255;code++)for(ci=0;ci<5;ci++)for(co=0;co<5;co++)for(pg=0;pg<3;pg++)for(m=0;m<4;m++){
        len=code==255?0:1+(code%5);
        memset(text_buffer,0xF1,sizeof text_buffer);
        for(k=0;k<len;k++)text_buffer[k]=(u_char)((code+k*31)%255);
        text_buffer[len]=255;
        x=coords[ci];y=coords[(ci+2)%5];color=colors[co];D_800A85D0=pages[pg];mutation=m;
        expected_ot=&tables[(code+ci)&1];bad=stage=row=0;
        func_80098F2C(expected_ot,text_buffer,x,y,color);
        if(bad||stage!=4+(int)len*3||row!=(int)len||text_buffer[len]!=255||text_buffer[len+1]!=0xF1){printf("FAIL glyph %u,%u,%u,%u,%u stage%d row%d\n",code,ci,co,pg,m,stage,row);return 1;}
        fixtures++;
    }
    /* All 255 drawable codes in one string exercise the running horizontal offset. */
    for(k=0;k<255;k++)text_buffer[k]=k;text_buffer[255]=255;
    x=-300;y=7;color=3;mutation=0;D_800A85D0=0;bad=stage=row=0;
    func_80098F2C(expected_ot,text_buffer,x,y,color);
    if(bad||stage!=769||row!=255)return 2;
    printf("PASS %u projected-text fixtures\n",fixtures+1);return 0;
}
