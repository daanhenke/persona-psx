#ifndef PERSONA_NAME_ENTRY_H
#define PERSONA_NAME_ENTRY_H

/* Persona 1 (JP) - the name-entry screen's state.
 *
 * Three fields are entered in turn. Each has room for eight cells, but only
 * the third is scanned that far: the first two stop at five. */
#include <types.h>

#define NAME_FIELDS 3
#define NAME_CELLS  8
#define NAME_SHORT  5   /* how far the first two fields are used */

typedef struct {
    /* 0x00 */ u_char  pad00[3];
    /* 0x03 */ u_char  field;                  /* which field is being edited */
    /* 0x04 */ u_char  pad04[4];
    /* 0x08 */ u_short text[NAME_FIELDS][NAME_CELLS];
    /* 0x38 */ char    cursor[NAME_FIELDS];    /* cell the caret sits on */
    /* 0x3B */ u_char  pad3B[9];
    /* 0x44 */ u_char  x[NAME_FIELDS];         /* where each field draws */
    /* 0x47 */ u_char  pad47[1];
    /* 0x48 */ u_char  y[NAME_FIELDS];
    /* 0x4B */ u_char  pad4B[1];
} NameEntry;

extern NameEntry g_name_entry;

#endif
