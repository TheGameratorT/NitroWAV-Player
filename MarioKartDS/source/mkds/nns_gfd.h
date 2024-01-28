#include <nds.h>

#ifndef _NNS_GFD_H_
#define _NNS_GFD_H_

#ifdef __cplusplus
extern "C"
{
#endif
typedef u32 NNSGfdTexKey;

typedef NNSGfdTexKey (*NNSGfdFuncAllocTexVram)(u32 szByte, BOOL is4x4comp, u32 opt);
typedef int (*NNSGfdFuncFreeTexVram)(NNSGfdTexKey key);

extern NNSGfdFuncAllocTexVram   NNS_GfdDefaultFuncAllocTexVram; 
extern NNSGfdFuncFreeTexVram    NNS_GfdDefaultFuncFreeTexVram;

static inline NNSGfdTexKey NNS_GfdAllocTexVram(u32 szByte, BOOL is4x4comp, u32 opt)
{
    return (*NNS_GfdDefaultFuncAllocTexVram)(szByte, is4x4comp, opt );
}

static inline int NNS_GfdFreeTexVram(NNSGfdTexKey memKey)
{
    return (*NNS_GfdDefaultFuncFreeTexVram)(memKey);
}

#define NNS_GFD_TEXKEY_ADDR_SHIFT 3

static inline u32 NNS_GfdGetTexKeyAddr( NNSGfdTexKey memKey )
{
    return (u32)( ( ( 0x0000FFFF & memKey ) ) << NNS_GFD_TEXKEY_ADDR_SHIFT );
}

typedef u32 NNSGfdPlttKey;

typedef NNSGfdPlttKey (*NNSGfdFuncAllocPlttVram)(u32 szByte, BOOL is4pltt, u32 opt);
typedef int (*NNSGfdFuncFreePlttVram)(NNSGfdPlttKey plttKey);

extern NNSGfdFuncAllocPlttVram  NNS_GfdDefaultFuncAllocPlttVram; 
extern NNSGfdFuncFreePlttVram   NNS_GfdDefaultFuncFreePlttVram; 

static inline NNSGfdPlttKey NNS_GfdAllocPlttVram(u32 szByte, BOOL is4pltt, u32 opt)
{
    return (*NNS_GfdDefaultFuncAllocPlttVram)( szByte, is4pltt, opt );
}

static inline int NNS_GfdFreePlttVram(NNSGfdPlttKey plttKey)
{
    return (*NNS_GfdDefaultFuncFreePlttVram)(plttKey);
}

#define NNS_GFD_PLTTKEY_ADDR_SHIFT 3

static inline u32 NNS_GfdGetPlttKeyAddr( NNSGfdPlttKey plttKey )
{
    return (u32)( ( 0x0000FFFF & plttKey )  << NNS_GFD_PLTTKEY_ADDR_SHIFT );
}


#ifdef __cplusplus
}
#endif

#endif