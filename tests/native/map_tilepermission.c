/* Substitute only the fixed map address; execute the production C unchanged. */
static union { unsigned long long alignment; unsigned char bytes[80032]; } storage;
#define PERSONA_TILE_MAP storage.bytes
#include "../../src/s2d/game/tilepermission.c"
extern int printf(const char *, ...);
short D_800B8FD4, D_800B8FD8;
u_char D_800B1D30[8];

int main(void)
{
    const short positions[][2] = {{0,0},{199,199},{0,199},{199,0},{37,112}};
    const int flags[] = {0,1,127,128,255,256,-1,-129};
    const u_char blink[] = {0,1,128,255};
    int p, a, b, bits, f, offset, expected, count=0, i;
    u_short tile;
    for(i=0;i<80032;i++)storage.bytes[i]=0xA5;
    *(int*)storage.bytes=16;
    for(i=0;i<8;i++)D_800B1D30[i]=0xC9;
    for(p=0;p<5;p++) for(a=0;a<4;a++) for(b=0;b<4;b++)
        for(bits=0;bits<8;bits++) for(f=0;f<8;f++) {
            D_800B8FD4=positions[p][0]; D_800B8FD8=positions[p][1];
            D_800B1D30[0]=blink[a]; D_800B1D30[1]=blink[b];
            offset=16+((199-positions[p][1])*200+positions[p][0])*2;
            tile=(u_short)((bits<<12)|0x8A5A);
            *(u_short*)(storage.bytes+offset)=tile;
            expected=(flags[f]&128) || !(bits&1) ||
                (!((bits&2)&&!blink[a]) && !((bits&4)&&!blink[b]));
            if(func_8008CA28(flags[f])!=expected)return 1;
            if(*(u_short*)(storage.bytes+offset)!=tile || *(int*)storage.bytes!=16)return 2;
            if(storage.bytes[offset-1]!=0xA5 || storage.bytes[offset+2]!=0xA5)return 3;
            if(D_800B8FD4!=positions[p][0] || D_800B8FD8!=positions[p][1])return 4;
            if(D_800B1D30[0]!=blink[a] || D_800B1D30[1]!=blink[b])return 5;
            for(i=2;i<8;i++)if(D_800B1D30[i]!=0xC9)return 6;
            storage.bytes[offset]=storage.bytes[offset+1]=0xA5;
            count++;
        }
    if(count!=5120)return 7;
    printf("%d native map-tile permission fixtures passed\n",count);return 0;
}
