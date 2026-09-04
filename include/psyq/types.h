#ifndef TYPES_H
#define TYPES_H

/* Base types as the Psy-Q SDK headers expect them. */
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;

typedef unsigned char      u_char;
typedef unsigned short     u_short;
typedef unsigned int       u_int;
typedef unsigned long      u_long;

typedef unsigned int       size_t;

/* Short forms used by the Psy-Q headers. */
typedef signed char        s8;
typedef short              s16;
typedef int                s32;
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;


#ifndef NULL
#define NULL 0
#endif

#endif /* TYPES_H */
