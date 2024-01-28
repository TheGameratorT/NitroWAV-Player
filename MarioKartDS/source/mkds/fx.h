#include <nds.h>

#ifndef __FX_H__
#define __FX_H__

#ifdef __cplusplus
extern "C"
{
#endif
	#define FX64C_65536_360                  ((fx64c) 0x000000b60b60b60bLL)    // 182.044444444 
	#define FX_DEG_TO_IDX(deg)            ((u16) ((FX64C_65536_360 * (deg) + 0x80000000000LL) >> 44))
	#define FX_Mul(v1, v2) FX_MulFunc(v1, v2)
	#define FX32_SHIFT 12

	extern fx16 FX_SinCosTable_[];
	
	s32 FX_DivS32(s32 a, s32 b);

	static inline fx16 FX_SinIdx(int idx)
	{
		return FX_SinCosTable_[((idx >> 4) << 1)];
	}
	
	static inline fx16 FX_CosIdx(int idx)
	{
		return FX_SinCosTable_[((idx >> 4) << 1) + 1];
	}
	
	fx32 FX_MulFunc(s32 v1, s32 v2);
	
	static inline s64 FX_Mul64(s64 v1, s64 v2) {
		return ((s64)v1 * (s64)v2 + 0x800LL) >> FX32_SHIFT;
	}
	
#ifdef __cplusplus
}
#endif

#endif