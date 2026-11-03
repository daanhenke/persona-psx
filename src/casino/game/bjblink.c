/* Blackjack natural-label animation and input activation. */
extern unsigned long g_casino_pad_trig;
extern unsigned char g_bj_play_step;
extern unsigned char g_bj_natural;
extern unsigned char g_bj_total[];
extern unsigned char g_bj_label_n[];
extern unsigned char natural_label_pending __asm__("D_800AAEA8+0x4");
extern int D_8009B2B4[];
extern char D_8009AD88[];
extern int g_casino_timer;
extern short g_casino_seqs[];
extern void SsSeqStop(short);
extern void SsSeqPlay(short, short, short);
extern void func_8007F368(int, int);
extern void func_8007EB20(int, unsigned char, int, unsigned char *, char *, int, int);

extern void func_800815D8(void);
extern void func_8007EF28(int), func_8007F00C(int),func_8007F160(int,int,int);
void func_80079AD8(void) {
 if(g_bj_natural==10 && g_casino_timer>=64){
  if((g_casino_timer&127)==0){
   func_8007EB20(10,g_bj_total[1],D_8009B2B4[3],g_bj_label_n+1,D_8009AD88,0,1);
   natural_label_pending=1;
  }else if(g_casino_timer%128==64){
   func_8007EB20(1,g_bj_total[1],D_8009B2B4[3],g_bj_label_n+1,D_8009AD88,0,1);
   natural_label_pending=0;
  }
 }
 if(g_casino_timer==0){func_8007EF28(0);func_8007F00C(112);}
 if(g_casino_timer<3)func_8007F160(112,1,g_casino_timer);
 if(g_casino_timer>=3)func_800815D8();
}

