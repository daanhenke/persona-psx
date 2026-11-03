/* Select and animate a blackjack result label or two-digit total. */
#include <persona/casino/casino.h>
extern short D_8009B114[], D_8009B128[], D_8009B13C[], D_8009B178[], D_8009B164[];
extern short D_8009B150[], D_8009B18C[], D_8009B1A0[], D_8009B1B4[], D_8009B1C8[];
extern short digits[] __asm__("D_8009B1EE-18");
extern void CasinoResetLayout(CasinoLayout *, int), CasinoPlaySeq(short *, u_long *, short);
extern void func_8007FC94(u_char, short), func_8007EE78(CasinoFrame *, short, short);
extern void CasinoTween(CasinoLayout *, int, short, short, short, short), CasinoTexLayout(CasinoLayout *);
void func_8007EB20(u_char type, u_char number, CasinoLayout *layout, u_char *count,
                   CasinoFrame *other, u_char sound, short frames){
        int width = 0;
        short delta = 0;
        unsigned char old, tens;
        CasinoResetLayout(layout, 1);
        switch (type){
    case 2:
        CasinoPlaySeq(&g_casino_seqs[3],(u_long*)0x13871c,g_casino_main_vab);
        width=24;
        layout->tex_idx=D_8009B114;
        old=*count;
        *count=4;
        delta=4-old;
        break;
    case 3:
        width=16;
        layout->tex_idx=D_8009B128;
        old=*count;
        *count=3;
        delta=3-old;
        break;
    case 4:
        width=24;
        layout->tex_idx=D_8009B13C;
        old=*count;
        *count=4;
        delta=4-old;
        break;
    case 5:
        width=24;
        layout->tex_idx=D_8009B178;
        old=*count;
        *count=4;
        delta=4-old;
        break;
    case 6:
        width=24;
        layout->tex_idx=D_8009B164;
        old=*count;
        *count=4;
        delta=4-old;
        break;
    case 7:
        width=24;
        layout->tex_idx=D_8009B150;
        old=*count;
        *count=4;
        delta=4-old;
        break;
    case 9:
        width=40;
        layout->tex_idx=D_8009B18C;
        old=*count;
        *count=6;
        delta=6-old;
        break;
    case 10:
        width=40;
        layout->tex_idx=D_8009B1A0;
        old=*count;
        *count=6;
        delta=6-old;
        if(sound)func_8007FC94(number,3);
        break;
    case 8:
        width=40;
        if(sound)CasinoPlaySeq(&g_casino_seqs[3],(u_long*)0x1386e0,g_casino_vab);
        layout->tex_idx=D_8009B1B4;
        old=*count;
        *count=6;
        delta=6-old;
        break;
    case 14:
        width=0;
        layout->tex_idx=D_8009B1C8;
        old=*count;
        *count=2;
        delta=2-old;
        break;
    case 1:
        tens=(unsigned char)(number%100)/10;
        if(tens){digits[8]=tens+30;
        width=8;
        }else{digits[8]=42;
        width=0;
        }old=*count;
        *count=2;
        delta=2-old;
        digits[9]=(unsigned char)(number%10)+30;
        layout->tex_idx=digits;
        if(sound)func_8007FC94(number,3);
        break;
}
        func_8007EE78(other, delta, frames);
        CasinoTween(layout, width, 0, 0, 0, 1);
        CasinoTween(layout, -8, 0, 16, 0, frames);
        CasinoTexLayout(layout);
}
