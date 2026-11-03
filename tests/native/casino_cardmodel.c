/* Retain the production C and use an ordinary host symbol for the PS1 bank. */
#define __asm__(symbol)
#include "../../src/casino/game/cardmodel.c"
#undef __asm__
extern int printf(const char *, ...);
CasinoCardBank card_bank;
static CasinoObj objects[256];
static CasinoFace faces[22];
static CasinoFaceDef defs[64];
static short index_rows[32][20], *rows[32];
static int chosen, kind_id, first_id, count_id, calls, bad;
static const short first_values[] = {-32768,-1,0,1,100,32767};

void CasinoBuildObj(CasinoModel *m)
{
    int i, j;
    calls++;
    if(m->def!=defs || m->tex || m->def_idx!=rows[kind_id+8] || m->tex_idx || m->obj!=&objects[chosen]) bad=1;
    if(objects[chosen].first!=first_values[first_id] || objects[chosen].n!=count_id) bad=1;
    for(i=0;i<22;i++) {
        if(faces[i].z!=(i>0&&i<21 ? defs[index_rows[kind_id+8][i-1]].z : 0xA55A)) bad=1;
        if(faces[i].on!=(i>0&&i<21 ? defs[index_rows[kind_id+8][i-1]].on : 0xB7)) bad=1;
        if(faces[i].pad!=0x5A) bad=1;
        for(j=0;j<4;j++) if(faces[i].v[j].vx!=123 || faces[i].v[j].vy!=-456 || faces[i].v[j].vz!=789 || faces[i].v[j].pad!=-1) bad=1;
    }
}
int main(void)
{
    const int kinds[]={-8,-1,0,7,23};
    const int counts[]={0,20,40};
    int i,j,k,n,count=0;
    card_bank.defs=defs;card_bank.indices=rows+8;card_bank.objects=objects;
    card_bank.textures=0;card_bank.texture_indices=0;
    for(i=0;i<64;i++) {defs[i].z=(u_short)(i*1031);defs[i].on=(u_char)(i*7);}
    for(i=0;i<32;i++) {rows[i]=index_rows[i];for(j=0;j<20;j++)index_rows[i][j]=(i*9+j*5)%64;}
    for(chosen=0;chosen<256;chosen++)for(k=0;k<5;k++)for(first_id=0;first_id<6;first_id++)for(n=0;n<3;n++) {
        kind_id=kinds[k];count_id=counts[n];
        for(i=0;i<256;i++) {objects[i].first=-222;objects[i].n=333;objects[i].face=0;}
        objects[chosen].face=faces+1;objects[chosen].n=count_id;
        for(i=0;i<22;i++) {
            faces[i].z=0xA55A;faces[i].on=0xB7;faces[i].pad=0x5A;
            for(j=0;j<4;j++) {faces[i].v[j].vx=123;faces[i].v[j].vy=-456;faces[i].v[j].vz=789;faces[i].v[j].pad=-1;}
        }
        calls=bad=0;
        func_800824CC(chosen,0x12345678,kind_id,first_values[first_id]);
        if(bad || calls!=1 || objects[chosen].face!=faces+1)return 1;
        for(i=0;i<256;i++) if(i!=chosen && (objects[i].first!=-222 || objects[i].n!=333 || objects[i].face))return 2;
        if(card_bank.defs!=defs || card_bank.indices!=rows+8 || card_bank.objects!=objects || card_bank.textures || card_bank.texture_indices)return 3;
        count++;
    }
    if(count!=23040)return 4;
    printf("%d native card-model fixtures passed\n",count);return 0;
}
