/* Draw the scene-image quad with the current map scale and restore projection offsets. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
extern int D_800AA73C[];
extern VECTOR D_800B5F2C;

void func_8009C478(GsOT *ot)
{
    MATRIX matrix;
    SVECTOR rotation;
    VECTOR translation;
    SVECTOR vertices[4];
    POLY_FT4 poly;
    long old_x, old_y, i;

    if (D_800AA73C[0x5C1C/4]) {
        SetPolyFT4(&poly);
        SetSemiTrans(&poly,0);
        SetShadeTex(&poly,1);
        poly.v1=poly.v0=0x80;
        poly.u0=0;poly.u1=0x58;poly.u2=0;poly.v2=0xE0;poly.u3=0x58;poly.v3=0xE0;
        poly.clut=GetClut(0x200,0xF3);
        poly.tpage=GetTPage(1,0,0x280,0x180);
        for(i=0;i<4;i++) {
            vertices[i].vx=(i&1)*0x58-0x2C;
            vertices[i].vy=(i/2)*0x60-0x30;
            vertices[i].vz=0;
        }
        ReadGeomOffset(&old_x,&old_y);
        SetGeomOffset(D_800AA73C[0x5C20/4]*100-196,-12);
        rotation.vz=rotation.vy=rotation.vx=0;
        translation.vy=translation.vx=0;translation.vz=640;
        RotMatrix(&rotation,&matrix);
        TransMatrix(&matrix,&translation);
        ScaleMatrix(&matrix,&D_800B5F2C);
        SetRotMatrix(&matrix);SetTransMatrix(&matrix);
        RotTransPers4(&vertices[0],&vertices[1],&vertices[2],&vertices[3],(long *)&poly.x0,(long *)&poly.x1,(long *)&poly.x2,(long *)&poly.x3,&i,&i);
        GsSortPoly(&poly,ot,0);
        SetGeomOffset(old_x,old_y);
    }
}
