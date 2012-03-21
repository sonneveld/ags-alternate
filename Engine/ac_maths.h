#ifndef _AC_MATHS_H_HEADER
#define _AC_MATHS_H_HEADER

// unfortunately MSVC and GCC automatically push floats as doubles
// to functions, thus we need to manually access it as 32-bit
#define SCRIPT_FLOAT(x) long __script_float##x
#define INIT_SCRIPT_FLOAT(x) float x = *((float*)&__script_float##x)
#define FLOAT_RETURN_TYPE long
#define RETURN_FLOAT(x) return *((long*)&x)

// MACPORT FIX 9/6/5: undef M_PI first
#undef M_PI
#define M_PI 3.14159265358979323846

extern void register_maths_script_functions();

#endif