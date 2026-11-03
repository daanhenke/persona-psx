/* Blackjack menu piece positions. The first record field points to word data. */
typedef struct {
    int *words;
} CasinoMenuPieceRecord;
extern CasinoMenuPieceRecord D_8009BBA8,D_8009BC20,D_8009BC98,D_8009BD10,D_8009BDA4,D_8009BE64,D_8009BF34,D_8009BFF4;
void func_8007F00C(unsigned char mode) {
    switch (mode) {
    case 0x73:
        D_8009BBA8.words[3] = 160;
        D_8009BC20.words[3] = 176;
        D_8009BC98.words[3] = 192;
        D_8009BD10.words[3] = 208;
        D_8009BDA4.words[3] = 160;
        D_8009BE64.words[3] = 176;
        D_8009BF34.words[3] = 192;
        D_8009BFF4.words[3] = 208;
        break;
    case 0x72:
        D_8009BBA8.words[3] = 168;
        D_8009BC20.words[3] = 184;
        D_8009BC98.words[3] = 200;
        D_8009BDA4.words[3] = 168;
        D_8009BE64.words[3] = 184;
        D_8009BF34.words[3] = 200;
        break;
    case 0x70:
    case 0x71:
        D_8009BBA8.words[3] = 176;
        D_8009BC20.words[3] = 192;
        D_8009BDA4.words[3] = 176;
        D_8009BE64.words[3] = 192;
        break;
    }
}

