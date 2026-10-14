#ifndef TYPES_H
#define TYPES_H

typedef unsigned char   u8;
typedef char            i8;
typedef unsigned short  u16;
typedef short           i16;
typedef unsigned int    u32;
typedef int             i32;
typedef u32             usize;
typedef i32             isize;
typedef _Bool           bool;
#define false           0
#define true            1

typedef enum ERROR_TYPE_T
{
    ERROR_TYPE_OK,
    ERROR_TYPE_INVALID_PARAM,
    ERROR_TYPE_UNSUPPORTED,
    ERROR_TYPE_I2C,
    ERROR_TYPE_TIMEOUT,
    ERROR_TYPE_NFC_NO_TAGS_FOUND
} ERROR_TYPE_T;

#endif // TYPES_H
