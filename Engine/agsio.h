#ifndef _ACSIO_H_HEADER
#define _AGSIO_H_HEADER


#include "stdint.h"
extern int fread_le_u32(uint32_t *result, FILE *fp);
extern int fread_le_u16(uint16_t *result, FILE *fp);
extern int fread_le_s32(int32_t *result, FILE *fp);
extern int fread_le_s16(int16_t *result, FILE *fp);

#endif