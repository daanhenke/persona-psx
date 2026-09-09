/* Persona 1 (JP) - the six-slot command picker.
 *
 * Each slot is a pair of objects. BtlPickSpawn makes the per-slot piece first
 * and the shared frame second, and it is the frame that ends up in
 * g_btl_pick_objs - so the per-slot piece is reached through its attached
 * link, and a setter called on the frame reaches both.
 *
 * The picker has several pages of six commands. g_btl_pick_page says which is
 * up, g_btl_pick_live says which of that page's slots may be chosen, and the
 * help line under the menu is that page's row of text.
 */
#ifndef PERSONA_BTLP_PICK_H
#define PERSONA_BTLP_PICK_H

#include <decomp/types.h>
#include <persona/btlp/object.h>

/* Slots to a page. */
#define BTL_PICK_SLOTS 6

/* How bright a slot is drawn, live or not, and how fast it gets there. */
#define PICK_DARK 0x20
#define PICK_LIVE 0x80
#define PICK_FADE 0xFF

/* Where the help line is put. */
#define PICK_HELP_X 0x10
#define PICK_HELP_Y 0x94

extern BtlObj      *g_btl_pick_objs[];
extern const u_char g_btl_pick_live[][BTL_PICK_SLOTS];
extern u_char       g_btl_pick_page;
extern const char  *g_btl_pick_help[];
extern short        g_btl_pick_help_row;
extern short        g_btl_pick_help_row2;
extern u_char       g_btl_no_help;

extern void BtlPickSpawn(void);
extern void BtlPickRefresh(void);
extern void BtlPickHighlight(int chosen);
extern int  BtlPickShowPage(int page);

#endif
