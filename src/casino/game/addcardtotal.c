/* Update low/high blackjack totals after drawing another card. */
extern int func_800823A0(unsigned char);
void func_800821B8(unsigned char *totals,unsigned char card){
 int value; unsigned char low=totals[0],high=totals[1];
 if((unsigned char)(card%13)==0){
  if(totals[0]==totals[1] && totals[0]+11<22){totals[0]=low+1;totals[1]+=11;}
  else{totals[0]=low+1;totals[1]+=1;}
  if(totals[0]>=22)totals[0]=totals[1];
  if(totals[1]>=22)totals[1]=totals[0];
 }else{
  value=func_800823A0(card);
  if(totals[0]+(unsigned char)value>=22)totals[0]=high+value;else totals[0]=value+totals[0];
  if(totals[1]+(unsigned char)value>=22)totals[1]=low+value;else totals[1]=value+totals[1];
 }
}
