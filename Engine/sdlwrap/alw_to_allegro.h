//
//  alw_to_allegro.h
//  EngineMac
//
//  Created by Nick Sonneveld on 20/04/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef EngineMac_alw_to_allegro_h
#define EngineMac_alw_to_allegro_h

#include <assert.h>

// for actual allegro code to use the wrapper stuff
#undef MIN
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))

#undef MAX
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))

#define AL_PI        3.14159265358979323846
#define _AL_SINCOS(x, s, c)  do { (c) = cos(x); (s) = sin(x); } while (0)

typedef ALW_BITMAP BITMAP;
typedef ALW_BLENDER_FUNC BLENDER_FUNC;
typedef ALW_COLOR_MAP COLOR_MAP;
typedef alw_fixed fixed;

#define AL_CONST const
#define ASSERT assert
#define _AL_MALLOC malloc

#define BMP_ID_MASK ALW_BMP_ID_MASK
#define BMP_ID_SUB ALW_BMP_ID_SUB
#define BMP_ID_LOCKED ALW_BMP_ID_SUB

#define acquire_bitmap alw_acquire_bitmap
#define bestfit_color alw_bestfit_color
#define bitmap_color_depth alw_bitmap_color_depth
#define bitmap_mask_color alw_bitmap_mask_color
#define bmp_read_line alw_bmp_read_line
#define bmp_read_line alw_bmp_read_line
#define bmp_select alw_bmp_select
#define bmp_unwrite_line alw_bmp_unwrite_line
#define bmp_write_line alw_bmp_write_line
#define getpixel alw_getpixel
#define is_linear_bitmap alw_is_linear_bitmap
#define is_memory_bitmap alw_is_memory_bitmap
#define is_same_bitmap alw_is_same_bitmap
#define makecol_depth alw_makecol_depth
#define release_bitmap alw_release_bitmap

#define getr8 alw_getr8
#define getg8 alw_getg8
#define getb8 alw_getb8
#define geta8 alw_geta8

#define getr15 alw_getr15
#define getg15 alw_getg15
#define getb15 alw_getb15
#define geta15 alw_geta15

#define getr16 alw_getr16
#define getg16 alw_getg16
#define getb16 alw_getb16
#define geta16 alw_geta16

#define getr24 alw_getr24
#define getg24 alw_getg24
#define getb24 alw_getb24
#define geta24 alw_geta24

#define getr32 alw_getr32
#define getg32 alw_getg32
#define getb32 alw_getb32
#define geta32 alw_geta32

#define makecol8 alw_makecol8
#define makecol15 alw_makecol15
#define makecol16 alw_makecol16
#define makecol24 alw_makecol24
#define makecol32 alw_makecol32

#define getr_depth alw_getr_depth
#define getg_depth alw_getg_depth
#define getb_depth alw_getb_depth
#define geta_depth alw_geta_depth

#define getpixel alw_getpixel
#define putpixel alw_putpixel

#define fixdiv alw_fixdiv
#define fixmul alw_fixmul

#define set_trans_blender alw_set_trans_blender

#define hline alw_hline
#define vline alw_vline

#endif
