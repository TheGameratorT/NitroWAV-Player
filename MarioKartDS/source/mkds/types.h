#include <nds.h>

#ifndef _TYPES_H
#define _TYPES_H

#define ATTRIBUTE_ALIGN(num) __attribute__ ((aligned(num)))

//

typedef bool BOOL;

//----------------------------------------------------------------------------
//  fx16: s3.12 fixed point number
//----------------------------------------------------------------------------
typedef s16 fx16;
#define FX16_SHIFT          12
#define FX16_INT_SIZE       3
#define FX16_DEC_SIZE       12

#define FX16_INT_MASK       0x7000
#define FX16_DEC_MASK       0x0fff
#define FX16_SIGN_MASK      0x8000

/* 7.999759  */
#define FX16_MAX            ((fx16)0x7fff)
/* -8        */
#define FX16_MIN            ((fx16)0x8000)

#define FX_FX16_TO_F32(x)    ((f32)((x) / (f32)(1 << FX16_SHIFT)))
#define FX_F32_TO_FX16(x)    ((fx16)(((x) > 0) ? \
                                     (fx16)((x) * (1 << FX16_SHIFT) + 0.5f ) : \
                                     (fx16)((x) * (1 << FX16_SHIFT) - 0.5f )))

#define FX16_CONST(x)       FX_F32_TO_FX16(x)

//fx32

typedef s32 fx32;
#define FX32_SHIFT          12
#define FX32_INT_SIZE       19
#define FX32_DEC_SIZE       12

#define FX32_INT_MASK       0x7ffff000
#define FX32_DEC_MASK       0x00000fff
#define FX32_SIGN_MASK      0x80000000

#define FX32_MAX            ((fx32)0x7fffffff)
#define FX32_MIN            ((fx32)0x80000000)

#define FX_MUL(v1, v2)       (fx32)(((fx64)(v1) * (v2) + 0x800LL) >> FX32_SHIFT)
#define FX_MUL32x64C(v1, v2) (fx32)(((v2) * (v1) + 0x80000000LL) >> 32)

#define FX_FX32_TO_F32(x)    ((f32)((x) / (f32)(1 << FX32_SHIFT)))
#define FX_F32_TO_FX32(x)    ((fx32)(((x) > 0) ? \
                                     ((x) * (1 << FX32_SHIFT) + 0.5f ) : \
                                     ((x) * (1 << FX32_SHIFT) - 0.5f )))

#define FX32_CONST(x)        FX_F32_TO_FX32(x)

#define FX32_ONE	4096

//----------------------------------------------------------------------------
//  fx64: s51.12 fixed point number
//----------------------------------------------------------------------------
typedef s64 fx64;
#define FX64_SHIFT          12
#define FX64_INT_SIZE       51
#define FX64_DEC_SIZE       12

#define FX64_INT_MASK       ((fx64)0x7ffffffffffff000)
#define FX64_DEC_MASK       ((fx64)0x0000000000000fff)
#define FX64_SIGN_MASK      ((fx64)0x8000000000000000)

#define FX64_MAX            ((fx64)0x7fffffffffffffff)
#define FX64_MIN            ((fx64)0x8000000000000000)

#define FX_FX64_TO_F32(x)   ((f32)((x) / (f32)(1 << FX64_SHIFT)))
#define FX_F32_TO_FX64(x)   ((fx64)(((x) > 0) ? \
                                    ((x) * (1 << FX32_SHIFT) + 0.5f ) : \
                                    ((x) * (1 << FX32_SHIFT) - 0.5f )))

#define FX64_CONST(x)       FX_F32_TO_FX64(x)

typedef s64 fx64c;
#define FX64C_SHIFT          32
#define FX64C_INT_SIZE       31
#define FX64C_DEC_SIZE       32

#define FX64C_INT_MASK       ((fx64c)0x7fffffff00000000)
#define FX64C_DEC_MASK       ((fx64c)0x00000000ffffffff)
#define FX64C_SIGN_MASK      ((fx64c)0x8000000000000000)

#define FX64C_MAX            ((fx64c)0x7fffffffffffffff)
#define FX64C_MIN            ((fx64c)0x8000000000000000)

#define FX_FX64C_TO_F32(x)   ((f32)((x) / (f32)((fx64c)1 << FX64C_SHIFT)))
#define FX_F32_TO_FX64C(x)   ((fx64c)(((x) > 0) ? \
                                      ((x) * ((fx64c)1 << FX64C_SHIFT) + 0.5f ) : \
                                      ((x) * ((fx64c)1 << FX64C_SHIFT) - 0.5f )))

#define FX64C_CONST(x)      FX_F32_TO_FX64C(x)

//GXRgb

typedef u16 GXRgb;
#define GX_RGB_R_SHIFT            (0)
#define GX_RGB_R_MASK             (0x001f)
#define GX_RGB_G_SHIFT            (5)
#define GX_RGB_G_MASK             (0x03e0)
#define GX_RGB_B_SHIFT            (10)
#define GX_RGB_B_MASK             (0x7c00)
#define GX_RGB(r, g, b)           ((GXRgb)(((r) << GX_RGB_R_SHIFT) | \
                                           ((g) << GX_RGB_G_SHIFT) | \
                                           ((b) << GX_RGB_B_SHIFT)))
//
typedef u32 GXSt;
//MtxFx44
typedef union
{
    struct
    {
        fx32    _00, _01, _02, _03;
        fx32    _10, _11, _12, _13;
        fx32    _20, _21, _22, _23;
        fx32    _30, _31, _32, _33;
    };
    fx32    m[4][4];
    fx32    a[16];
}
MtxFx44;
//

typedef u8 REGType8;
typedef u16 REGType16;
typedef u32 REGType32;
typedef u64 REGType64;

typedef vu8 REGType8v;
typedef vu16 REGType16v;
typedef vu32 REGType32v;
typedef vu64 REGType64v;

#endif