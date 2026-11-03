#include <persona/casino/blackjack.h>
extern int printf(const char *, ...);
extern void *memset(void *, int, unsigned long);
extern void *memcpy(void *, const void *, unsigned long);
extern int memcmp(const void *, const void *, unsigned long);
void func_80079C28(void);
void func_8007BA60(void);

/* The assembly reads the first hand through both cards and cards - 1. */
unsigned char native_cards[17];
__asm__(".global g_bj_cards,D_800A967F\n.set D_800A967F,native_cards\n"
        ".set g_bj_cards,native_cards+1\n");
unsigned char g_bj_count[2], g_bj_total[2], g_bj_split_total[2];
unsigned char g_bj_natural[2], g_bj_label_n[3], g_bj_card_obj, g_bj_play_step;
unsigned char D_800A8818, D_800A8674, D_800AAEA8[8];
int g_casino_timer;
short g_casino_main_vab, g_casino_seqs[16];
CasinoObj g_casino_objs[25];
CasinoLayoutDef D_8009B2B4[4], D_8009AD88[1];
CasinoFrame D_8009AE2C, D_8009AED0;
static CasinoLayout layouts[4];

typedef struct {
    int timer;
    unsigned char step, hand, object, option, count[2], cards[17];
    unsigned char total[2], split[2], natural[2], labels[3], pending[8];
} State;
enum { DRAW, ANIM, SEQUENCE, HIDE, SPRITES, TOTAL, CLASS, RANK, PAIR, STATUS, LABEL };
typedef struct { int kind, a, b, c, d; } Event;
static Event expected[128];
static unsigned mode, scenario, recording, bad, n, at, rank_calls;
static const unsigned char totals[][2] = {
    {0,0}, {20,20}, {21,21}, {21,11}, {11,21}, {22,22}, {255,255}, {1,21}
};
static const int ranks[][2] = {
    {10,9}, {9,9}, {0,9}, {0,0}, {266,9}, {256,265}, {-1,-1}, {10,10}
};

static State get_state(void)
{
    State s; memset(&s,0,sizeof s);
    s.timer=g_casino_timer; s.step=g_bj_play_step; s.hand=D_800A8674;
    s.object=g_bj_card_obj; s.option=D_800A8818;
    memcpy(s.count,g_bj_count,2); memcpy(s.cards,native_cards,17);
    memcpy(s.total,g_bj_total,2); memcpy(s.split,g_bj_split_total,2);
    memcpy(s.natural,g_bj_natural,2); memcpy(s.labels,g_bj_label_n,3);
    memcpy(s.pending,D_800AAEA8,8); return s;
}
static void put_state(const State *s)
{
    g_casino_timer=s->timer; g_bj_play_step=s->step; D_800A8674=s->hand;
    g_bj_card_obj=s->object; D_800A8818=s->option;
    memcpy(g_bj_count,s->count,2); memcpy(native_cards,s->cards,17);
    memcpy(g_bj_total,s->total,2); memcpy(g_bj_split_total,s->split,2);
    memcpy(g_bj_natural,s->natural,2); memcpy(g_bj_label_n,s->labels,3);
    memcpy(D_800AAEA8,s->pending,8);
}
static int call(State *s,int kind,int a,int b,int c,int d)
{
    Event e={kind,a,b,c,d}; int ret=0;
    if(recording){if(n==128){bad=1;return 0;} expected[n++]=e;}
    else {if(at>=n||memcmp(&e,&expected[at],sizeof e))bad=1; at++;}
    switch(kind){
    case DRAW:
        if(s->count[b]<8)s->count[b]++;
        s->cards[1+8*b+s->count[b]-1]=(unsigned char)(scenario+37);
        if(s->object)s->object--;
        if(mode&8)s->timer=32;
        break;
    case ANIM: if((mode&2)&&b==32&&c==2048)s->timer=65; break;
    case SEQUENCE: if(mode&1)s->timer=36; break;
    case TOTAL:
        memcpy(a?s->split:s->total,totals[scenario%8],2);
        if(mode&4)s->hand^=1;
        break;
    case CLASS: ret=scenario%3==0?3:scenario%3; break;
    case RANK: ret=ranks[scenario%8][rank_calls++!=0]; break;
    case PAIR: ret=scenario%4==0?258:scenario%4; break;
    case STATUS:
        s->natural[b]=(unsigned char)(1+scenario%10);
        if(mode&4)s->hand^=1;
        break;
    case LABEL: s->labels[c]++; if(mode&8)s->timer=77; break;
    }
    return ret;
}
static int callback(int kind,int a,int b,int c,int d)
{State s=get_state();int r=call(&s,kind,a,b,c,d);put_state(&s);return r;}
void func_8007E878(unsigned char side,unsigned char face,unsigned char *cards,unsigned char *count)
{
    int h=cards==g_bj_cards[0]?0:cards==g_bj_cards[1]?1:-1;
    if(face||h<0||count!=g_bj_count+h)bad=1;
    callback(DRAW,side,h,0,0);
}
void CasinoStartAnim(CasinoObj *p,short frames,short rx,short ry,short rz,
                     long tx,long ty,long tz,long sx,long sy,long sz)
{
    if(p<g_casino_objs||p>=g_casino_objs+25||rx||rz||ty||tz||sx||sy||sz)bad=1;
    callback(ANIM,(int)(p-g_casino_objs),frames,ry,(int)tx);
}
void CasinoPlaySeq(short *seq,u_long *data,short bank)
{if(seq!=g_casino_seqs||(unsigned long)data!=0x138830||bank!=g_casino_main_vab)bad=1;callback(SEQUENCE,0,0,0,0);}
void CasinoObjSetOn(CasinoObj *p,short type,short flag,unsigned char on)
{if(type!=2||flag!=17||on||p<g_casino_objs||p>=g_casino_objs+25)bad=1;callback(HIDE,(int)(p-g_casino_objs),0,0,0);}
void CasinoSpritesSetOn(short start,short count,int on)
{if(count!=3||on)bad=1;callback(SPRITES,start,0,0,0);}
void func_800821B8(unsigned char *p,unsigned char card)
{int h=p==g_bj_total?0:p==g_bj_split_total?1:-1;if(h<0)bad=1;callback(TOTAL,h,card,0,0);}
unsigned char func_80081F80(unsigned char *p)
{if(p!=g_bj_cards[0])bad=1;return (unsigned char)callback(CLASS,0,0,0,0);}
int func_80082034(int count){return callback(RANK,count,0,0,0);}
int func_8008230C(unsigned char *p)
{if(p!=g_bj_total)bad=1;return callback(PAIR,0,0,0,0);}
void func_8007F960(unsigned char hand,void *cards,unsigned char *total)
{
    int h=cards==g_bj_cards[0]?0:cards==g_bj_cards[1]?1:-1;
    if(h<0||total!=(h?g_bj_split_total:g_bj_total))bad=1;
    callback(STATUS,hand,h,0,0);
}
void func_8007EB20(unsigned char type,unsigned char value,CasinoLayout *layout,
                  unsigned char *count,CasinoFrame *frame,unsigned char sound,short frames)
{
    int l=layout==&layouts[1]?1:layout==&layouts[2]?2:layout==&layouts[3]?3:-1;
    int c=count==&g_bj_label_n[1]?1:count==&g_bj_label_n[2]?2:-1;
    CasinoFrame *want=l==1?(CasinoFrame *)D_8009AD88:l==2?&D_8009AE2C:&D_8009AED0;
    if(l<0||c<0||sound!=1||frames!=8||frame!=want)bad=1;
    callback(LABEL,type,value,c,l);
}
static void animate(State *s,int object,int frames,int ry,int x)
{call(s,ANIM,object,frames,ry,x);}
static void hide(State *s,int object,int group)
{
    int i;call(s,HIDE,object,0,0,0);
    if(group)for(i=0;i<4;i++)call(s,SPRITES,group+10*i,0,0,0);
}
/* Reference phases are taken from the MIPS branches and callback arguments.
   In particular timer tests reload after calls, and split hand zero falls
   through at tick 36. The final split dispatch snapshots the selected hand. */
static void reference(State *s,int split)
{
    int i,h,k;
    if(s->timer==0){
        h=split?s->hand:0;
        if(h<2)call(s,DRAW,split?h+1:0,h,0,0);
        animate(s,s->object+1,32,0,-320);
    }
    if(s->timer==32){
        call(s,SEQUENCE,0,0,0,0);animate(s,s->object+1,32,2048,0);
        if(s->timer==32){
            h=split?s->hand:0;
            if(h<2){
                animate(s,h?22:24,4,0,-4);
                animate(s,split?(h?19:20):22,4,0,-4);
                i=split?(h?20-s->count[0]:18):20;
                while(i>s->object+1){animate(s,i,4,0,-4);i--;}
            }
        }
    }
    if(split&&s->timer==36){
        h=s->hand;
        if(h==0)hide(s,s->count[0]==3?20:22-s->count[0],s->count[0]==3?60:0);
        if(h<2)hide(s,s->count[1]==3?22:22-s->count[0]-s->count[1],s->count[1]==3?67:0);
    }
    if(s->timer<65)return;
    if(split){
        h=s->hand;s->step=0x1B;s->timer=-1;
        if(h<2){
            call(s,TOTAL,h,s->cards[8*h+s->count[h]],0,0);
            call(s,STATUS,s->hand,h,0,0);
            call(s,LABEL,s->natural[h],h?s->split[1]:s->total[1],h+1,h+2);
        }
        s->step=0x21;return;
    }
    hide(s,s->count[0]==3?22:24-s->count[0],s->count[0]==3?63:0);
    call(s,TOTAL,0,s->cards[s->count[0]],0,0);
    s->step=0x1B;s->timer=-1;
    if(call(s,CLASS,0,0,0,0)==3){s->step=0x71;s->natural[0]=3;}
    if(s->option==0){
        k=call(s,RANK,s->count[0],0,0,0)&255;
        if(k==10){
            s->natural[0]=10;
            if(s->total[0]==21&&s->total[1]==21)s->step=0x71;
            else{s->step=0x1B;s->pending[4]=1;}
        }else{
            k=call(s,RANK,s->count[0],0,0,0)&255;
            if(k==9){s->step=0x71;s->natural[0]=9;}
            else if(s->total[0]==21&&s->total[1]==21&&s->natural[0]==1)s->step=0x71;
        }
    }
    if((call(s,PAIR,0,0,0,0)&255)==2){s->step=0x71;s->natural[0]=2;}
    if(s->option==1&&s->natural[0]==1)s->step=0x71;
    call(s,LABEL,s->natural[0],s->total[1],1,1);
}
static int check(State *initial,int split,unsigned long id)
{
    State want=*initial,actual;
    bad=0;recording=1;n=0;at=0;rank_calls=0;reference(&want,split);
    put_state(initial);recording=0;rank_calls=0;
    if(split)func_8007BA60();else func_80079C28();
    actual=get_state();
    if(bad||at!=n||memcmp(&want,&actual,sizeof want)){
        printf("FAIL fixture %lu split=%d timer=%d counts=%d,%d object=%d hand=%d mode=%u scenario=%u events=%u/%u bad=%u\n",
            id,split,initial->timer,initial->count[0],initial->count[1],initial->object,initial->hand,mode,scenario,at,n,bad);
        return 1;
    }
    return 0;
}
int main(void)
{
    static const int timers[]={-1,0,31,32,33,35,36,37,64,65,66,2147483647};
    static const unsigned char objects[]={0,3,10,18,23},hands[]={0,1,2,255};
    State s;unsigned t,c,d,o,h,q,i;unsigned long cases=0;
    for(i=0;i<4;i++)D_8009B2B4[i].l=&layouts[i];g_casino_main_vab=13;
    for(t=0;t<12;t++)for(c=1;c<=8;c++)for(o=0;o<5;o++)for(q=0;q<3;q++)
    for(mode=0;mode<16;mode++)for(scenario=0;scenario<12;scenario++){
        memset(&s,0,sizeof s);s.timer=timers[t];s.count[0]=c;s.count[1]=4;
        s.object=objects[o];s.option=q;s.step=0xAB;s.hand=1;
        s.natural[0]=scenario%4;s.natural[1]=7;s.labels[1]=255;
        for(i=0;i<17;i++)s.cards[i]=(unsigned char)(i*13+scenario);
        for(i=0;i<8;i++)s.pending[i]=0xA5;
        if(check(&s,0,++cases))return 1;
    }
    for(t=0;t<12;t++)for(c=1;c<=8;c++)for(d=1;d<=8;d++)for(o=0;o<5;o++)for(h=0;h<4;h++)
    for(mode=0;mode<16;mode++)for(scenario=0;scenario<4;scenario++){
        memset(&s,0,sizeof s);s.timer=timers[t];s.count[0]=c;s.count[1]=d;
        s.object=objects[o];s.hand=hands[h];s.option=2;s.step=0xAC;
        s.natural[0]=3;s.natural[1]=7;s.labels[2]=255;
        for(i=0;i<17;i++)s.cards[i]=(unsigned char)(i*17+scenario);
        if(check(&s,1,++cases))return 1;
    }
    printf("PASS %lu blackjack hit controller fixtures\n",cases);return 0;
}
