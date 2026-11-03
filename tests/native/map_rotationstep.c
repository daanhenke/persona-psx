#include <persona/s2d/s2d.h>
extern int puts(const char *);
extern void func_8008BA9C(void);
short D_800B0D3C,D_800B0D40,D_800B0D38,g_s2d_facing;
int g_key_lock;
S2dXform g_map_xform;
int main(void) {
    static const short times[]={-32768,-1,0,1,2,32767};
    static const short speeds[]={-32768,-1,0,1,32767};
    static const short angles[]={-32768,-1,0,32767};
    int t,s,a;
    for(t=0;t<6;t++) for(s=0;s<5;s++) for(a=0;a<4;a++) {
        short expected=(short)((unsigned short)angles[a]+(unsigned short)(speeds[s]*128));
        D_800B0D3C=times[t];D_800B0D40=speeds[s];D_800B0D38=-32768;
        g_s2d_facing=19;g_key_lock=7;
        g_map_xform.rot.vx=123;g_map_xform.rot.vy=angles[a];g_map_xform.rot.vz=-123;
        g_map_xform.trans.vx=999;g_map_xform.scale.vz=888;
        func_8008BA9C();
        if(times[t]>0) {
            if(D_800B0D3C!=times[t]-1 || g_map_xform.rot.vy!=expected ||
               g_key_lock!=(times[t]==1) || g_s2d_facing!=(times[t]==1?-32768:19)) return 1;
        } else if(D_800B0D3C!=times[t] || g_map_xform.rot.vy!=angles[a] || g_key_lock!=7 || g_s2d_facing!=19) return 2;
        if(D_800B0D40!=speeds[s] || D_800B0D38!=-32768 || g_map_xform.rot.vx!=123 ||
           g_map_xform.rot.vz!=-123 || g_map_xform.trans.vx!=999 || g_map_xform.scale.vz!=888) return 3;
    }
    puts("120 native map-rotation fixtures passed");return 0;
}
