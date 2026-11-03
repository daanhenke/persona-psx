typedef struct { short v[12][5]; } Table;

const Table D_80065284 = {{{546,547,539,550,551},
    {549,548,539,553,552},
    {542,545,539,554,557},
    {543,544,539,555,556},
    {537,542,543,538,538},
    {537,545,544,538,538},
    {540,554,555,541,541},
    {540,557,556,541,541},
    {537,546,549,540,540},
    {537,547,548,540,540},
    {538,550,553,541,541},
    {538,551,552,541,541}}};
extern unsigned char D_800A85BC[][5];
extern void CasinoSpritesSetOn(int,int,unsigned char);
void func_80084E4C(signed char row,unsigned char active,signed char offset)
{
    Table sprites=D_80065284;
    int i;

    for(i=0;i<5;i++)
    {
        if(D_800A85BC[row+offset][i] == 1)
            CasinoSpritesSetOn(sprites.v[row][i],1,active);
    }
}
