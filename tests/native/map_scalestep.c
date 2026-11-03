/* Host symbols retain the production C and its constant local pointers. */
#define __asm__(symbol)
#include "../../src/s2d/game/mapscalestep.c"
#undef __asm__
extern int printf(const char *, ...);
MapScale map_scale;
long scale_state;

int main(void)
{
    const long values[] = {-4096,-1,0,1,383,384,385,3711,3712,3713,4095,4096,4097,8192};
    const long states[] = {-1,0,1,2,3,4,2147483647};
    long x,y,state;
    int i,j,k,frame,count=0;
    for(i=0;i<14;i++)for(j=0;j<14;j++)for(k=0;k<7;k++) {
        x=values[i];y=values[j];state=states[k];
        map_scale.x=x;map_scale.y=y;map_scale.z=0x13579BDF;scale_state=state;
        for(frame=0;frame<40;frame++) {
            if(state==1) {
                x=x+384>4096 ? 4096 : x+384;
                y=y+384>4096 ? 4096 : y+384;
                if(x>=4096 && y>=4096)state=2;
            } else if(state==3) {
                x=x-384<0 ? 0 : x-384;
                y=y-384<0 ? 0 : y-384;
                if(x<=0 && y<=0)state=0;
            }
            func_8009C36C();
            if(map_scale.x!=x || map_scale.y!=y || map_scale.z!=0x13579BDF || scale_state!=state)return 1;
            count++;
        }
    }
    if(count!=54880)return 2;
    printf("%d native map-scale frame checks passed\n",count);return 0;
}
