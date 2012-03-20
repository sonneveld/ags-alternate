#include <stdlib.h>
#include <stdio.h>

#include "stdint.h"

// read little endian 32 bit from file
int fread_le_u32(uint32_t *result, FILE *fp)
{
    uint8_t buf[4];
    size_t count = fread(buf, 4, 1, fp);
    if (count != 1)
        return -1;

    uint32_t r;
    r  = buf[0];
    r |= buf[1] <<  8;
    r |= buf[2] << 16;
    r |= buf[3] << 24;
    *result = r;

    return 0;
}

// read little endian 16 bit from file
int fread_le_u16(uint16_t *result, FILE *fp)
{
    uint8_t buf[2];
    size_t count = fread(buf, 2, 1, fp);
    if (count != 1)
        return -1;

    uint16_t r;
    r  = buf[0];
    r |= buf[1] <<  8;
    *result = r;

    return 0;
}

int fread_le_s32(int32_t *result, FILE *fp) {
    uint32_t uresult;
    if (fread_le_u32(&uresult, fp))
        return -1;
    *result = (int32_t)uresult;
    return 0;
}

int fread_le_s16(int16_t *result, FILE *fp) {
    uint16_t uresult;
    if (fread_le_u16(&uresult, fp))
        return -1;
    *result = (int16_t)uresult;
    return 0;
}