/* Draw three transformed gradient panels into an ordering table. */
#include <persona/s2d/s2d.h>
/* The three 40-byte records updated by func_80099A00. */
typedef struct {
    short state;
    u_short flags;
    short x, y, w, h;
    SVECTOR rotation;
    VECTOR scale;
    long rate;
} S2dGradientPanel;
extern S2dGradientPanel D_800B5E28[3];
extern VECTOR D_8006509C;
void func_80099AD8(GsOT *ot,int priority)
{
    POLY_G4 poly;
    MATRIX matrix;
    VECTOR translation=D_8006509C;
    SVECTOR vertices[4];
    long j;
    int i,ox,oy;
    S2dGradientPanel *p;
    SetPolyG4(&poly);SetSemiTrans(&poly,1);SetShadeTex(&poly,0);p=D_800B5E28;
    poly.r0=0;poly.g0=0;poly.b0=0;
    poly.r1=0;poly.g1=0;poly.b1=0;
    poly.r2=96;poly.g2=64;poly.b2=255;
    poly.r3=96;poly.g3=64;poly.b3=255;
    /* Mode 3 preserves the previous offsets; the first panel must use 0..2. */
    for(i=0;i<3;p++,i++) {
        SetGeomOffset(p->x,p->y);
        switch((p->flags>>1)&3){case 0:ox=0;oy=0;break;case 1:ox=-(p->w>>1);oy=-(p->h>>1);break;case 2:ox=-p->w;oy=0;break;}
        for(j=0;j<4;j++){vertices[j].vx=ox+(j&1)*p->w;vertices[j].vy=oy+(j>>1)*p->h;vertices[j].vz=0;}
        RotMatrix(&p->rotation,&matrix);TransMatrix(&matrix,&translation);ScaleMatrix(&matrix,&p->scale);
        SetRotMatrix(&matrix);SetTransMatrix(&matrix);
        RotTransPers4(vertices,vertices+1,vertices+2,vertices+3,(long*)&poly.x0,(long*)&poly.x1,(long*)&poly.x2,(long*)&poly.x3,&j,&j);
        GsSortPoly(&poly,ot,priority);
    }
}
