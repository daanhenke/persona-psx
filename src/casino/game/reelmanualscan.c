/* Persona 1 (JP) - test manual reel lines and show accepted symbols. */
typedef struct
{
    short row[12][5];
} ReelSprites;
extern ReelSprites   D_80065284;
extern unsigned char D_800A85BC[12][5];
extern signed char   D_800A85F8[];
extern unsigned char D_800AF9D0[], D_800AF9D4[], D_800AF9D8[], D_800AF9DC[], g_casino_bet;
extern void          CasinoSpritesSetOn(short, short, int), func_80087818(int, int, int, int);
static inline void   ShowLine(signed char lane)
{
    ReelSprites sprites = D_80065284;
    int         k;
    for (k = 0; k < 5; k++)
        if (D_800A85BC[lane][k] == 1)
            CasinoSpritesSetOn(sprites.row[lane][k], 1, 1);
}
void func_80085F08(void)
{
    signed char   lines[12][5];
    int           lanes = g_casino_bet;
    int           lane;
    unsigned char symbol;
    int           n, count, i;
    CasinoSpritesSetOn(537, 21, 0);
    if (lanes >= 5)
        lanes += lanes - 4;
    lines[0][0]  = D_800AF9D4[0];
    lines[1][0]  = D_800AF9D4[3];
    lines[0][1]  = D_800AF9D4[1];
    lines[1][1]  = D_800AF9D4[2];
    lines[0][3]  = D_800AF9D8[0];
    lines[1][3]  = D_800AF9D8[3];
    lines[0][4]  = D_800AF9D8[1];
    lines[1][4]  = D_800AF9D8[2];
    lines[2][0]  = D_800AF9D0[0];
    lines[3][0]  = D_800AF9D0[1];
    lines[2][1]  = D_800AF9D0[3];
    lines[3][1]  = D_800AF9D0[2];
    lines[2][3]  = D_800AF9DC[0];
    lines[3][3]  = D_800AF9DC[1];
    lines[2][4]  = D_800AF9DC[3];
    lines[3][4]  = D_800AF9DC[2];
    lines[4][1]  = D_800AF9D0[0];
    lines[5][1]  = D_800AF9D0[3];
    lines[4][2]  = D_800AF9D0[1];
    lines[5][2]  = D_800AF9D0[2];
    lines[5][4]  = -1;
    lines[4][4]  = -1;
    lines[6][1]  = D_800AF9DC[0];
    lines[7][1]  = D_800AF9DC[3];
    lines[6][2]  = D_800AF9DC[1];
    lines[7][2]  = D_800AF9DC[2];
    lines[7][4]  = -1;
    lines[6][4]  = -1;
    lines[8][1]  = D_800AF9D4[0];
    lines[9][1]  = D_800AF9D4[1];
    lines[8][2]  = D_800AF9D4[3];
    lines[9][2]  = D_800AF9D4[2];
    lines[9][4]  = -1;
    lines[8][4]  = -1;
    lines[10][1] = D_800AF9D8[0];
    lines[11][1] = D_800AF9D8[1];
    lines[10][2] = D_800AF9D8[3];
    lines[11][2] = D_800AF9D8[2];
    lines[11][4] = -1;
    lines[10][4] = -1;
    for (lane = 0; lane < lanes; lane++)
    {
        for (symbol = 0; symbol < 7; symbol++)
        {
            lines[0][2] = lines[1][2] = lines[2][2] = lines[3][2] = lines[4][0] = lines[4][3] = lines[5][0] = lines[5][3] = lines[6][0] = lines[6][3] = lines[7][0] = lines[7][3] = lines[8][0] = lines[8][3] = lines[9][0] = lines[9][3] = lines[10][0] = lines[10][3] = lines[11][0] = lines[11][3] = symbol;
            D_800A85F8[lane]                                                                                                                                                                                                                                                                          = 0;
            n                                                                                                                                                                                                                                                                                         = lane < 4 ? 5 : 4;
            count                                                                                                                                                                                                                                                                                     = 0;
            for (i = 0; i < n; i++)
            {
                if (lines[lane][i] == symbol)
                {
                    count++;
                    D_800A85BC[lane][i] = 1;
                }
                else
                    D_800A85BC[lane][i] = 0;
            }
            if (count >= 3)
            {
                func_80087818(symbol, count, n, lane);
                if (lane < 4)
                    D_800A85BC[lane][2] = 0;
                else
                    D_800A85BC[lane][0] = D_800A85BC[lane][3] = 0;
                if (D_800A85F8[lane] >= 3)
                {
                    ShowLine(lane);
                }
            }
        }
    }
}