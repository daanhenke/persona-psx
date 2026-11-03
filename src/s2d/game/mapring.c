/* Build a deformed 64-edge ring and queue its triangles and backdrop. */
#include <persona/s2d/s2d.h>
typedef struct { POLY_G3 poly; DVECTOR points[65]; } RingScratch;
typedef struct { DR_MODE modes[2]; POLY_F4 back[2]; POLY_G3 triangles[2][64]; } RingPackets;
extern RingPackets D_800B5F5C;
extern struct { DVECTOR centre[4]; int frame; } D_800A566C;
extern int ring_frame __asm__("D_800A566C+16");
extern void *memmove(void *,const void *,unsigned int);
#ifndef PERSONA_RING_SCRATCH
#define PERSONA_RING_SCRATCH ((RingScratch *)0x1F800000)
#endif
void func_80096A38(GsOT *ot)
{
 RingScratch *s=PERSONA_RING_SCRATCH;
 POLY_F4 *back;
 int count,i,light;DVECTOR *point;
 SetPolyG3(&s->poly);SetSemiTrans(&s->poly,1);SetShadeTex(&s->poly,0);
 for(i=0;i<64;i++){
  s->points[i].vx=(unsigned)(rsin(i*64)*5)>>6;
  s->points[i].vy=(unsigned)(rcos(i*64)*15)>>9;
 }
 count=i;
 for(i=0;i<32;i++){
  s->points[i].vx-=(((rsin(i*199+ring_frame*96)+4096)/2)*64)>>12;
  s->points[63-i].vx+=(((rsin(i*199+ring_frame*96+2048)+4096)/2)*64)>>12;
 }
 s->points[count].vx=s->points[0].vx;
 s->points[count].vy=s->points[0].vy;
 point=s->points;
 for(i=0;i<64;i++){
  s->poly.x0=point->vx;s->poly.y0=point->vy;
  s->poly.x1=D_800A566C.centre[i/16].vx;s->poly.y1=D_800A566C.centre[i/16].vy;
  s->poly.x2=point[1].vx;s->poly.y2=point[1].vy;
  s->poly.r0=0;s->poly.g0=0;s->poly.b0=0;
  s->poly.r1=255;s->poly.g1=207;s->poly.b1=175;
  s->poly.r2=0;s->poly.g2=0;s->poly.b2=0;
  memmove(&D_800B5F5C.triangles[g_draw_side][i],s,sizeof(s->poly));
  point++;
  AddPrim(ot->org,&D_800B5F5C.triangles[g_draw_side][i]);
 }
 back=&D_800B5F5C.back[g_draw_side];
 SetPolyF4(back);SetSemiTrans(back,1);SetShadeTex(back,0);
 light=(((rsin(ring_frame*64)+4096)/2)*56)>>12;
 back->r0=light+24;back->g0=light+8;back->b0=light;
 back->x0=-256;back->y0=-120;back->x1=256;back->y1=-120;
 back->x2=-256;back->y2=120;back->x3=256;back->y3=120;
 AddPrim(ot->org,back);
 SetDrawMode(&D_800B5F5C.modes[g_draw_side],0,0,32,0);
 AddPrim(ot->org,&D_800B5F5C.modes[g_draw_side]);
}
