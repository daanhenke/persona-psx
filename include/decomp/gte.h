#ifndef _DECOMP_GTE_H
#define _DECOMP_GTE_H

/* GAS versions of the Psy-Q projection intrinsics. inline.h emits placeholders
 * for Sony's assembler to expand; GAS needs the actual instruction words.
 * Keep the SDK's separate volatile statements and scratch-register clobbers:
 * gcc 2.6 uses those boundaries when allocating the surrounding C code.
 */
#define GTE_WORD(word) \
    __asm__ volatile (".word " #word : : : "$12", "$13", "$14", "$15", "memory")

#define gte_ldv0(r1) { \
    __asm__ volatile ("move  $12,%0" : : "r"(r1) : "$12", "$13", "$14", "$15", "memory"); \
    GTE_WORD(0xC9800000); /* lwc2 $0, 0($12) */ \
    GTE_WORD(0xC9810004); /* lwc2 $1, 4($12) */ \
}

#define gte_ldv3(r1, r2, r3) { \
    __asm__ volatile ("move  $12,%0" : : "r"(r1) : "$12", "$13", "$14", "$15", "memory"); \
    __asm__ volatile ("move  $13,%0" : : "r"(r2) : "$12", "$13", "$14", "$15", "memory"); \
    __asm__ volatile ("move  $14,%0" : : "r"(r3) : "$12", "$13", "$14", "$15", "memory"); \
    GTE_WORD(0xC9800000); /* lwc2 $0, 0($12) */ \
    GTE_WORD(0xC9810004); /* lwc2 $1, 4($12) */ \
    GTE_WORD(0xC9A20000); /* lwc2 $2, 0($13) */ \
    GTE_WORD(0xC9A30004); /* lwc2 $3, 4($13) */ \
    GTE_WORD(0xC9C40000); /* lwc2 $4, 0($14) */ \
    GTE_WORD(0xC9C50004); /* lwc2 $5, 4($14) */ \
}

#define gte_rtps() { \
    GTE_WORD(0x00000000); \
    GTE_WORD(0x00000000); \
    GTE_WORD(0x4A180001); /* RTPS */ \
}

#define gte_rtpt() { \
    GTE_WORD(0x00000000); \
    GTE_WORD(0x00000000); \
    GTE_WORD(0x4A280030); /* RTPT */ \
}

#define gte_stsxy(r1) { \
    __asm__ volatile ("move  $12,%0" : : "r"(r1) : "$12", "$13", "$14", "$15", "memory"); \
    GTE_WORD(0xE98E0000); /* swc2 $14, 0($12) */ \
}

#define gte_stsxy3(r1, r2, r3) { \
    __asm__ volatile ("move  $12,%0" : : "r"(r1) : "$12", "$13", "$14", "$15", "memory"); \
    __asm__ volatile ("move  $13,%0" : : "r"(r2) : "$12", "$13", "$14", "$15", "memory"); \
    __asm__ volatile ("move  $14,%0" : : "r"(r3) : "$12", "$13", "$14", "$15", "memory"); \
    GTE_WORD(0xE98C0000); /* swc2 $12, 0($12) */ \
    GTE_WORD(0xE9AD0000); /* swc2 $13, 0($13) */ \
    GTE_WORD(0xE9CE0000); /* swc2 $14, 0($14) */ \
}

#endif
