/* Blackjack shoe drawing and sequence restart helpers. */
extern short g_casino_seqs[];
extern unsigned char D_800B0AF8[],g_casino_step,g_bj_shoe_pos,g_casino_deck[],g_bj_reshuffle;
extern int g_casino_timer;
extern void SsSeqStop(short),SsSeqPlay(short,short,short),func_800821B8(unsigned char *,unsigned char);
void D_80081C60(void){SsSeqStop(g_casino_seqs[4]);SsSeqPlay(g_casino_seqs[4],1,1);D_800B0AF8[3]=119;g_casino_step=122;g_casino_timer=-1;}
void func_80081CC8(unsigned char *cards,unsigned char *count){cards[*count]=g_casino_deck[g_bj_shoe_pos];g_bj_shoe_pos++;if(g_bj_shoe_pos>=207){g_bj_shoe_pos=0;g_bj_reshuffle=1;}}
void func_80081D30(unsigned char *cards,unsigned char *count,unsigned char *totals){cards[*count]=g_casino_deck[g_bj_shoe_pos];g_bj_shoe_pos++;if(g_bj_shoe_pos>=207){g_bj_shoe_pos=0;g_bj_reshuffle=1;}func_800821B8(totals,cards[*count]);}

