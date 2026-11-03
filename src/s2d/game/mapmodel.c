/* Persona 1 (JP): build the two-part map model and its primitive/vertex lists.
 * Address expressions retain the original 32-bit PS1 layout.
 */

typedef struct
{
    int vertices;
    int nvertices;
    int normals;
    int nnormals;
    int primitives;
    int nprimitives;
    int scale;
} Model;
typedef struct
{
    int id;
    int flags;
    int nmodels;
    Model m[2];
} Tmd;
typedef struct
{
    unsigned char ilen;
    unsigned char olen;
    unsigned char flag;
    unsigned char mode;
    unsigned char color[4][4];
    short index[4];
} Prim;
typedef struct
{
    short x;
    short y;
    short z;
    short pad;
} Vertex;
extern struct
{
    char pad[20];
    unsigned char indices[7][4];
    Vertex normals[6];
} D_800A566C;
void *func_80097508(Tmd *t)
{
    int i;
    short *out;
    Prim *p;
    Vertex *v;
    unsigned char *ind;
    unsigned char *row;
    Vertex *normal_base;
    t->id = 65;
    t->flags = 0;
    t->nmodels = 2;
    t->m[0].vertices = 252;
    t->m[0].nvertices = 8;
    t->m[0].normals = t->m[0].vertices + 128;
    t->m[0].nnormals = 6;
    t->m[0].primitives = 56;
    t->m[0].nprimitives = 4;
    t->m[0].scale = 0;
    t->m[1].vertices = t->m[0].vertices + 64;
    t->m[1].nvertices = 8;
    t->m[1].normals = t->m[0].normals;
    t->m[1].nnormals = 6;
    t->m[1].primitives = 168;
    t->m[1].nprimitives = 3;
    t->m[1].scale = 0;
    p = (Prim *)((int)t + t->m[0].primitives + 12);
    for (i = 0; i < 7; i++)
    {
        row = D_800A566C.indices[i];
        p->ilen = 8;
        p->olen = 6;
        p->flag = 1;
        p->mode = 57;
        out = p->index;
        ind = row;
        do
        {
            *(out++) = *(ind++);
        } while (((int)ind) < ((int)(row + 4)));
        p++;
    }

    v = (Vertex *)((int)t + t->m[0].vertices + 12);
    normal_base = D_800A566C.normals;
    for (i = 0; i < 8; i++)
    {
        v->x = ((i & 1) * 2560) - 960;
        v->y = (-((1 - ((i / 2) & 1)) << 10)) * 3;
        v->z = (((i / 4) & 1) * 1280) - 960;
        v++;
    }

    for (i = 0; i < 8; i++)
    {
        v->x = ((i & 1) * 1280) - 320;
        v->y = (-((1 - ((i / 2) & 1)) << 10)) * 3;
        v->z = (((i / 4) & 1) * 1280) - 960;
        v++;
    }

    v = (Vertex *)((int)t + t->m[0].normals + 12);
    for (i = 0; i < 6; i++)
    {
        v->x = normal_base[i].x;
        v->y = normal_base[i].y;
        v->z = normal_base[i].z;
        v++;
    }

    return (((char *)t) + t->m[0].normals) + 60;
}
