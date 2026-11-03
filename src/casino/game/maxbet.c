/* Fill the selected bet in ten-token increments. */
extern signed char D_800AF73C[],g_casino_cursor[];
extern unsigned int g_casino_money,D_800A96A4[];
extern short g_casino_seqs[],g_casino_main_vab;
extern void SsSeqStop(short),SsSeqPlay(short,short,short),CasinoPlaySeq(short*,unsigned long*,short);
void func_80090330(void){
 int index=g_casino_cursor[1];
 if(D_800AF73C[0]){
 D_800AF73C[0]=0;
 if(g_casino_money>=10){
 if(D_800A96A4[index]<100){
 SsSeqStop(g_casino_seqs[6]);SsSeqPlay(g_casino_seqs[6],1,1);
 while(g_casino_money>=10 && D_800A96A4[index]<100){
 D_800A96A4[index]+=10;g_casino_money-=10;
 }
 }
 }else CasinoPlaySeq(g_casino_seqs,(unsigned long *)0x13B1F8,g_casino_main_vab);
 }
}
