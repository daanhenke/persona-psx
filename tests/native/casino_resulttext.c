/* Exercise real result-label C, including all byte-valued kinds and totals. */
#define __asm__(symbol)
#include "../../src/casino/game/resulttext.c"
#undef __asm__
extern int printf(const char *, ...);
extern void *memset(void *, int, unsigned long);
short g_casino_seqs[16], g_casino_main_vab, g_casino_vab;
short D_8009B114[10], D_8009B128[10], D_8009B13C[10], D_8009B178[10];
short D_8009B164[10], D_8009B150[10], D_8009B18C[10], D_8009B1A0[10];
short D_8009B1B4[10], D_8009B1C8[10], digits[12];
static CasinoLayout layout;
static CasinoFrame frames_data[4];
static u_char count;
static int events[8], event_count, bad, variant, kind, total, sound_on;
static int expected_n, expected_delta, expected_width;
static short duration, initial_texture[2];
static short *expected_texture;

static void record(int event) { if (event_count >= 8) bad=1; else events[event_count++]=event; }
void CasinoResetLayout(CasinoLayout *p, int mode)
{
    record(1); if(p != &layout || mode != 1) bad=1;
    if(variant==1) count=255;
}
void CasinoPlaySeq(short *handle, u_long *sequence, short vab)
{
    record(2);
    if(handle!=g_casino_seqs+3 || sequence!=(u_long *)(unsigned long)(kind==2?0x13871c:0x1386e0)
       || vab!=(kind==2?g_casino_main_vab:g_casino_vab)) bad=1;
    if(layout.tex_idx!=initial_texture) bad=1;
    if(variant==2) count=0;
}
void func_8007FC94(u_char value, short channel)
{
    record(3);
    if(value!=total || channel!=3 || count!=expected_n || layout.tex_idx!=expected_texture) bad=1;
    if(variant==2) count=77;
}
void func_8007EE78(CasinoFrame *p, short distance, short time)
{
    record(4);
    if(p!=frames_data || distance!=expected_delta || time!=duration) bad=1;
    if(layout.tex_idx!=expected_texture) bad=1;
}
void CasinoTween(CasinoLayout *p, int dx, short dy, short mode, short unused, short time)
{
    int first=event_count>0 && events[event_count-1]==4;
    record(first?5:6);
    if(p!=&layout || dx!=(first?expected_width:-8) || dy || mode!=(first?0:16)
       || unused || time!=(first?1:duration)) bad=1;
}
void CasinoTexLayout(CasinoLayout *p) {record(7);if(p!=&layout)bad=1;}

int main(void)
{
    static const int sizes[15]={0,2,4,3,4,4,4,4,6,6,6,0,0,0,2};
    static const int widths[15]={0,0,24,16,24,24,24,24,40,40,40,0,0,0,0};
    static const short times[]={-32768,-1,0,1,16,32767};
    short *labels[15];
    int i, fixtures=0, old, expected_events[8], n, expected_final;
    memset(labels,0,sizeof(labels));
    labels[1]=digits;labels[2]=D_8009B114;labels[3]=D_8009B128;
    labels[4]=D_8009B13C;labels[5]=D_8009B178;labels[6]=D_8009B164;
    labels[7]=D_8009B150;labels[8]=D_8009B1B4;labels[9]=D_8009B18C;
    labels[10]=D_8009B1A0;labels[14]=D_8009B1C8;
    for(kind=0;kind<256;kind++) for(total=0;total<256;total++) for(variant=0;variant<3;variant++) {
        memset(&layout,0,sizeof(layout));layout.tex_idx=initial_texture;layout.first=-1234;layout.busy=567;
        count=(u_char)(total*17+kind*31);old=variant==1?255:count;
        sound_on=(total+variant)%3;duration=times[(kind+total+variant)%6];
        g_casino_main_vab=-32768;g_casino_vab=32767;
        for(i=0;i<12;i++)digits[i]=-12345;
        expected_n=kind<15 && sizes[kind]?sizes[kind]:old;
        expected_width=kind<15?widths[kind]:0;
        expected_texture=kind<15 && labels[kind]?labels[kind]:initial_texture;
        n=0;expected_events[n++]=1;
        if(kind==2 || (kind==8 && sound_on)) {
            expected_events[n++]=2;if(variant==2)old=0;
        }
        expected_delta=kind<15 && sizes[kind]?sizes[kind]-old:0;
        if(kind==1)expected_width=total%100>=10?8:0;
        expected_final=expected_n;
        if((kind==1 || kind==10) && sound_on) {
            expected_events[n++]=3;if(variant==2)expected_final=77;
        }
        expected_events[n++]=4;expected_events[n++]=5;expected_events[n++]=6;expected_events[n++]=7;
        event_count=bad=0;
        func_8007EB20(kind,total,&layout,&count,frames_data,sound_on,duration);
        if(bad || event_count!=n || count!=expected_final || layout.tex_idx!=expected_texture || layout.first!=-1234 || layout.busy!=567) {
            printf("label failed kind=%d total=%d variant=%d bad=%d count=%d expected=%d\n",kind,total,variant,bad,count,expected_final);return 1;
        }
        for(i=0;i<n;i++)if(events[i]!=expected_events[i])return 2;
        for(i=0;i<12;i++) {
            int expected=-12345;
            if(kind==1 && i==8)expected=total%100>=10?(total%100)/10+30:42;
            if(kind==1 && i==9)expected=total%10+30;
            if(digits[i]!=expected)return 3;
        }
        fixtures++;
    }
    printf("%d native blackjack-label fixtures passed\n",fixtures);
    return fixtures==196608?0:4;
}
