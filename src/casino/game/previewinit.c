/* Initialize the first preview object's rotation, position and scale. */
#include <persona/casino/casino.h>

typedef struct
{
    double x, y, z;
} CasinoDoubleVector;
typedef struct
{
    CasinoXform        current, previous;
    CasinoDoubleVector r, p, s;
} PreviewState;
extern PreviewState  pair __asm__("D_800B0720");
extern unsigned char D_800B0800[];
extern CasinoModel   models[] __asm__("D_800A2BC8+14816");
extern void          func_80092C44(CasinoDoubleVector, short*);
extern void          func_80092CB0(CasinoDoubleVector, int*);
extern void          func_80092D34(CasinoDoubleVector, int*);
extern void          func_80093B7C(CasinoXform*);
extern void          func_80092DEC(CasinoModel*, SVECTOR, VECTOR, VECTOR);
void                 func_800925DC(void)
{
    pair.r.x      = 0.0;
    pair.r.y      = 0.0;
    pair.r.z      = 0.0;
    pair.p.x      = 24.0;
    pair.p.y      = -100.0;
    pair.p.z      = 634.0;
    pair.s.x      = 1.0;
    pair.s.y      = 1.0;
    pair.s.z      = 1.0;
    D_800B0800[0] = 0;
    D_800B0800[1] = 0;
    D_800B0800[2] = 0;
    D_800B0800[7] = 0;
    D_800B0800[8] = 1;
    func_80092C44(pair.r, &pair.current.rot.vx);
    func_80092C44(pair.r, &pair.previous.rot.vx);
    func_80092CB0(pair.p, &pair.current.trans.vx);
    func_80092CB0(pair.p, &pair.previous.trans.vx);
    func_80092D34(pair.s, &pair.current.scale.vx);
    func_80092D34(pair.s, &pair.previous.scale.vx);
    func_80093B7C(&pair.current);
    func_80092DEC(models, pair.current.rot, pair.current.trans, pair.current.scale);
}
