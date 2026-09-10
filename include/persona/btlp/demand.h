#ifndef PERSONA_BTLP_DEMAND_H
#define PERSONA_BTLP_DEMAND_H

#include <decomp/types.h>

#define BTL_DEMAND_KINDS 4

/* Twelve halfwords per demand: the opening script is entry 0, response
   scripts use entries 2..6, and their mood/weight pairs use entries 7..11. */
typedef struct
{
    u_short entry[12];
} BtlDemandRow;

/* The scene header's offset selects this area of the loaded script pack. */
typedef struct
{
    u_char       pad00[0x508];
    BtlDemandRow rows[BTL_DEMAND_KINDS];
} BtlDemandScene;

extern int           g_btl_demand_kind;
extern int           g_btl_demand_money;
extern int           g_btl_demand_item;
extern int           g_btl_demand_answer;
extern short         g_btl_demand_odds[];
extern short         g_btl_demand_roll;
extern const short   g_btl_demand_odds_by_kind[][BTL_DEMAND_KINDS];
extern const u_char* g_btl_demand_menus[];
extern const u_char* g_btl_talk_no_money_script;
extern const u_char* g_btl_talk_no_charm_script;

extern void BtlTalkSceneDemand(void);

#endif
