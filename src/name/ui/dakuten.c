/* Persona 1 (JP) - the voicing marks.  NAME @ 0x80066B70.
 *
 * The ゛ and ゜ keys change the character under the caret rather than typing
 * one: a kana code is taken relative to its page's first code, and the voiced
 * forms sit at fixed distances from the plain ones.
 */
#include <decomp/types.h>
#include <persona/name/entry.h>

#define CARET_CHAR g_name_text[g_name_field][g_name_cursor[g_name_field]]

int NameAddHandakuten(u_short code);

/* Voices `code` for the ゛ key (or hands the ゜ key, column 10, on).
   Answers 1 when the character takes no mark. */
int NameAddDakuten(u_short code, signed char col)
{
    u_short rel;

    rel = code - g_name_code_base[g_name_page];
    if (col == 10) {
        return NameAddHandakuten(code);
    }
    if (rel < 5 || rel >= 30 || (rel >= 20 && rel < 25)) {
        if (code == 0x54) {
            CARET_CHAR = 0x80;
        } else if (rel >= 0x2E && rel <= g_name_page + 0x41) {
            if (code == 0x80) {
                CARET_CHAR = 0x54;
            } else {
                CARET_CHAR = code - (u_short)(g_name_page + 0x29);
                if (rel >= g_name_page + 0x3D) {
                    CARET_CHAR += 5;
                }
            }
        } else if (rel >= g_name_page + 0x42 && rel <= g_name_page + 0x46) {
            CARET_CHAR -= 5;
        } else {
            return 1;
        }
    } else if (code == 0x54) {
        CARET_CHAR = 0x80;
    } else {
        CARET_CHAR = code + (u_short)(g_name_page + 0x29);
        if (rel >= 25 && rel < 30) {
            CARET_CHAR -= 5;
        }
    }
    NameDrawCursor();
    return 0;
}

/* The ゜ key. */
int NameAddHandakuten(u_short code)
{
    u_short rel;

    rel = code - g_name_code_base[g_name_page];
    if (rel < 25 || rel >= 30) {
        if (rel >= g_name_page + 0x42 && rel <= g_name_page + 0x46) {
            CARET_CHAR = code - (u_short)(g_name_page + 0x29);
        } else if (rel >= g_name_page + 0x3D && rel <= g_name_page + 0x41) {
            CARET_CHAR += 5;
        } else {
            return 1;
        }
    } else {
        CARET_CHAR = code + (u_short)(g_name_page + 0x29);
    }
    NameDrawCursor();
    return 0;
}
