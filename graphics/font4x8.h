#ifndef FONT4X8_H
#define FONT4X8_H

#define FONT4X8_DIGITS_FIRST '0'
#define FONT4X8_DIGITS_COUNT 10

#define FONT4X8_LOWER_FIRST  'a'
#define FONT4X8_LOWER_COUNT  26

static const uint8_t font4x8_digits[FONT4X8_DIGITS_COUNT][4] = {
    { // 0
        0b01111111,
        0b01000001,
        0b01000001,
        0b01111111
    },
    { // 1
        0b01111111,
        0b00000000,
        0b00000000,
        0b00000000
    },
    { // 2
        0b01001111,
        0b01001001,
        0b01001001,
        0b01111001
    },
    { // 3
        0b01111111,
        0b01001001,
        0b01001001,
        0b01001001
    },
    { // 4
        0b01111000,
        0b00001000,
        0b00001000,
        0b01111111
    },
    { // 0
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    },
    { // 0
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    },
    { // 0
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    },
    { // 0
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    },
    { // 0
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    },
};

static inline const uint8_t* font4x8_glyph(char a_c)
{
    if (a_c >= '0' && a_c <= '9')
        return font4x8_digits[(uint8_t)(a_c - '0')];
    return (const uint8_t *)0;
}

#endif // FONT4X8_H
