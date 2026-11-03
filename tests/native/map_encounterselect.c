/* The production selector, with synthetic records and a scripted random source. */
static unsigned char scene_flag;
#define PERSONA_ENCOUNTER_FLAG scene_flag
#define rand scripted_rand
#include "../../src/s2d/game/encounterselect.c"
#undef rand
extern int printf(const char *, ...);
extern void *memset(void *, int, unsigned long);
extern int memcmp(const void *, const void *, unsigned long);
EncounterChances D_80065050;S2dFade g_fade;
int D_800B9528,D_800B5F3C,g_key_lock,D_800B94EC[4];
u_char D_800A4CE4;short g_map_id,g_s2d_exit;
struct {u_char padding[0x5C28];u_short *record;} D_800AA73C;
static u_short records[3][31];
typedef struct {
    int fade,active,timer,reset,lock,record;short exit,map;u_char source,flag;EncounterChances chances;
} State;
typedef struct {int event;State state;} Event;
static Event actual[4],expected[4];static int used,bad,mutation,pending,roll,selection,random_calls;
static State read_state(void)
{
    State s;int i;memset(&s,0,sizeof(s));s.fade=g_fade.unk08;s.active=encounter_active;
    s.timer=D_800B9528;s.reset=D_800B5F3C;s.lock=g_key_lock;s.exit=g_s2d_exit;s.map=g_map_id;
    s.source=D_800A4CE4;s.flag=scene_flag;s.chances=D_80065050;s.record=-1;
    for(i=0;i<3;i++)if(encounter==records[i])s.record=i;return s;
}
static void write_state(State s)
{
    g_fade.unk08=s.fade;encounter_active=s.active;D_800B9528=s.timer;D_800B5F3C=s.reset;
    g_key_lock=s.lock;g_s2d_exit=s.exit;g_map_id=s.map;D_800A4CE4=s.source;scene_flag=s.flag;
    D_80065050=s.chances;encounter=records[s.record];
}
static void event(State *s,Event *e,int id)
{
    int i;memset(e,0,sizeof(*e));e->event=id;e->state=*s;
    if(mutation==1 && id==2)s->source=255;
    if(mutation==2 && id==2)s->record=2;
    if(mutation==3)for(i=0;i<4;i++)s->chances.value[i]=0;
}
static void callback(int id)
{
    State s=read_state();if(used>=4){bad=1;return;}event(&s,&actual[used++],id);write_state(s);
}
void *func_80096198(void){callback(1);return records[1];}
int scripted_rand(void){int r=(pending || random_calls)?selection:roll;callback(2);random_calls++;return r;}
static State model(State s,int *count,int *result)
{
    int n=0;EncounterChances copy=s.chances;*result=0;
    if(s.fade==1){
        if(s.active==1){
            event(&s,&expected[n++],2);s.reset=0;s.exit=2;s.flag=s.source;
            s.map=(short)records[s.record][selection%30];
        }else{
            s.timer++;if(s.timer>16)s.timer=16;
            if(s.timer==16){
                event(&s,&expected[n++],1);s.record=1;event(&s,&expected[n++],2);
                if(roll%32<copy.value[records[s.record][30]]){
                    s.active=1;event(&s,&expected[n++],2);s.reset=0;s.timer=0;s.lock=0;
                    s.exit=2;s.flag=s.source;s.map=(short)records[s.record][selection%30];*result=1;
                }
            }
        }
    }
    *count=n;return s;
}
int main(void)
{
    static const int fades[]={-1,0,1,2},timers[]={-3,0,14,15,16,17,2147483646},picks[]={0,29,30,32767};
    static const short chances[]={-1,0,16,33};
    int f,a,t,category,p,i,j,n,result,returned,fixtures=0;State initial,want,got;
    for(f=0;f<4;f++)for(a=0;a<4;a++)for(t=0;t<7;t++)for(category=0;category<4;category++)
    for(roll=0;roll<32;roll++)for(p=0;p<4;p++)for(mutation=0;mutation<4;mutation++){
        memset(&initial,0,sizeof(initial));initial.fade=fades[f];initial.active=fades[a];initial.timer=timers[t];
        initial.reset=1234;initial.lock=5678;initial.exit=-1;initial.map=-1234;initial.source=category*73;initial.flag=73;
        for(i=0;i<4;i++)initial.chances.value[i]=chances[i];
        for(i=0;i<3;i++){for(j=0;j<30;j++)records[i][j]=(u_short)(i*20000+j*977);records[i][30]=(category+i)%4;}
        selection=picks[p];pending=initial.fade==1 && initial.active==1;
        want=model(initial,&n,&result);write_state(initial);used=bad=random_calls=0;
        g_fade.rgb[0]=77;g_fade.rgb[1]=88;g_fade.rgb[2]=99;g_fade.unk04=111;
        D_800B94EC[0]=123;D_800B94EC[1]=456;D_800B94EC[3]=789;
        returned=func_80096224();got=read_state();
        if(bad || used!=n || returned!=result || memcmp(actual,expected,n*sizeof(Event)) || memcmp(&got,&want,sizeof(State))){
            printf("encounter failed fixture=%d mutation=%d result=%d expected=%d\n",fixtures,mutation,returned,result);return 1;
        }
        if(g_fade.rgb[0]!=77 || g_fade.rgb[1]!=88 || g_fade.rgb[2]!=99 || g_fade.unk04!=111 || D_800B94EC[0]!=123 || D_800B94EC[1]!=456 || D_800B94EC[3]!=789)return 2;
        fixtures++;
    }
    printf("%d native encounter-selector fixtures passed\n",fixtures);return fixtures==229376?0:3;
}
