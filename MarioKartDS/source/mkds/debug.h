#include <nds.h>

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef __cplusplus
extern "C"
{
#endif

	static inline void Break();
	void nocashPrint(const char* txt);
	void nocashPrint1(const char* txt, u32 r0);
	void nocashPrint2(const char* txt, u32 r0, u32 r1);
	void nocashPrint3(const char* txt, u32 r0, u32 r1, u32 r2);

	static inline void Break()
	{
		asm volatile("mov r11, r11");
	}
	
#ifdef __cplusplus
}
#endif

#endif