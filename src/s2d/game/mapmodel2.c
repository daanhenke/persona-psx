/* Persona 1 (JP): construct two gradient-colored model quads. */

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
int func_80097964(Tmd *t)
{
    int i;
    int j;
    Prim *p;
    Vertex *v;
    unsigned char (*col)[4];
    short *idx;
    t->id = 65;
    t->flags = 0;
    t->nmodels = 2;
    t->m[0].vertices = 112;
    t->m[0].nvertices = 4;
    t->m[0].normals = t->m[0].vertices + 64;
    t->m[0].nnormals = 1;
    t->m[0].primitives = 56;
    t->m[0].nprimitives = 1;
    t->m[0].scale = 0;
    t->m[1].vertices = t->m[0].vertices + 32;
    t->m[1].nvertices = 4;
    t->m[1].normals = t->m[0].normals;
    t->m[1].nnormals = 1;
    t->m[1].primitives = 84;
    t->m[1].nprimitives = 1;
    t->m[1].scale = 0;
    p = (Prim *)((int)t + t->m[0].primitives + 12);
    p->ilen = 8;
    p->olen = 6;
    p->flag = 1;
    p->mode = 57;
    p->color[0][3] = 57;
    idx = p->index;
    col = p->color;
    for (i = 0; i < 4; i++)
    {
        (*col)[0] = (1 - i / 2) * 255;
        (*col)[1] = (1 - i / 2) * 255;
        (*col)[2] = (1 - i / 2) * 255;
        *idx++ = i;
        col++;
    }
    p++;
    p->ilen = 8;
    p->olen = 6;
    p->flag = 1;
    p->mode = 57;
    p->color[0][3] = 57;
    idx = p->index;
    col = p->color;
    for (i = 0; i < 4; i++)
    {
        if (i != 3)
            (*col)[0] = 255;
        else
            (*col)[0] = 0;
        if (i != 3)
            (*col)[1] = 255;
        else
            (*col)[1] = 0;
        if (i != 3)
            (*col)[2] = 255;
        else
            (*col)[2] = 0;
        *idx++ = i;
        col++;
    }
    v = (Vertex *)((int)t + t->m[0].vertices + 12);
    for (j = 0; j < 4; j++)
    {
        v->x = (j & 1) * 2560 - 960;
        v->y = -((1 - j / 2) << 10);
        v->z = 960 - (j / 2) * 2560;
        v++;
    }
    for (j = 0; j < 4; j++)
    {
        v->x = (j & 1) * 2560 - 960;
        if (j != 3)
            v->y = -1024;
        else
            v->y = 0;
        v->z = 960 - (j / 2) * 2560;
        v++;
    }
    return 0;
}
