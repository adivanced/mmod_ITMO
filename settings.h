#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define dimensions 2  // amount of dimensions of expected data (ASM supports up to 8)
#define numbertype float // number format used in calculations
#define USE_ASM 0 // 0 - plain C++ version, 1 - optimized AVX2 assembly version of the idw function
#define interpolation_points 2 // points to interpolate by, can also be supplied dynamically as a program's argument

#endif
