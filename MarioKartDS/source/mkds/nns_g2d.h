#include <nds.h>

#ifndef _NNS_G2D_H
#define _NNS_G2D_H

typedef enum NNSG2dTextBGWidth
{
    NNS_G2D_TEXT_BG_WIDTH_256   = 32,
    NNS_G2D_TEXT_BG_WIDTH_512   = 64
}
NNSG2dTextBGWidth;

typedef enum NNSG2dCharaColorMode
{
    NNS_G2D_CHARA_COLORMODE_16 = 4,
    NNS_G2D_CHARA_COLORMODE_256 = 8
}
NNSG2dCharaColorMode;

typedef struct NNSG2dFont
{
    void* pRes;//NNSG2dFontInformation* pRes;            // Pointer to the expanded font resource
    /*NNSiG2dSplitCharCallback*/void* cbCharSpliter; // Pointer to the character string encoding process callback
    u16 isOldVer;
    u16 widthsSize;
}
NNSG2dFont;

struct NNSG2dCharCanvas;

typedef void (*NNSiG2dDrawGlyphFunc)(
    const struct NNSG2dCharCanvas* pCC,
	const NNSG2dFont* pFont,
    int x,
    int y,
    int cl,
	const void* pGlyph//const NNSG2dGlyph* pGlyph
);

typedef void (*NNSiG2dClearFunc)(
    const struct NNSG2dCharCanvas* pCC,
    int cl
);

typedef void (*NNSiG2dClearAreaFunc)(
    const struct NNSG2dCharCanvas* pCC,
    int cl,
    int x,
    int y,
    int w,
    int h
);

typedef struct NNSG2dCharCanvas
{
    u8* charBase;
    int areaWidth;
    int areaHeight;
    u8 dstBpp;
    u8 reserved[3];
    u32 param;
    NNSiG2dDrawGlyphFunc pDrawGlyph;
    NNSiG2dClearFunc pClear;
    NNSiG2dClearAreaFunc pClearArea;
}
NNSG2dCharCanvas;

typedef enum NNSG2dVerticalOrigin
{
    NNS_G2D_VERTICALORIGIN_TOP      = 0x1,
    NNS_G2D_VERTICALORIGIN_MIDDLE   = 0x2,
    NNS_G2D_VERTICALORIGIN_BOTTOM   = 0x4
}
NNSG2dVerticalOrigin;

typedef enum NNSG2dHorizontalOrigin
{
    NNS_G2D_HORIZONTALORIGIN_LEFT   = 0x8,
    NNS_G2D_HORIZONTALORIGIN_CENTER = 0x10,
    NNS_G2D_HORIZONTALORIGIN_RIGHT  = 0x20
}
NNSG2dHorizontalOrigin;

typedef enum NNSG2dVerticalAlign
{
    NNS_G2D_VERTICALALIGN_TOP       = 0x40,
    NNS_G2D_VERTICALALIGN_MIDDLE    = 0x80,
    NNS_G2D_VERTICALALIGN_BOTTOM    = 0x100
}
NNSG2dVerticalAlign;

typedef enum NNSG2dHorizontalAlign
{
    NNS_G2D_HORIZONTALALIGN_LEFT    = 0x200,
    NNS_G2D_HORIZONTALALIGN_CENTER  = 0x400,
    NNS_G2D_HORIZONTALALIGN_RIGHT   = 0x800
}
NNSG2dHorizontalAlign;

typedef struct NNSG2dTextCanvas
{
    const NNSG2dCharCanvas* pCanvas;
    const NNSG2dFont* pFont;
    int hSpace;
    int vSpace;
}
NNSG2dTextCanvas;

#ifdef __cplusplus
extern "C"
{
#endif
	
	void NNS_G2dCharCanvasInitForBG(NNSG2dCharCanvas* pCC, void* charBase, int areaWidth, int areaHeight, NNSG2dCharaColorMode colorMode);
	void NNS_G2dMapScrToCharText(void* scnBase, int areaWidth, int areaHeight, int areaLeft, int areaTop, NNSG2dTextBGWidth scnWidth, int charNo, int cplt);
	static inline void NNS_G2dTextCanvasInit(NNSG2dTextCanvas* pTxn, const NNSG2dCharCanvas* pCC, const NNSG2dFont* pFont, int hSpace, int vSpace);
	//void NNSi_G2dTextCanvasDrawString(const NNSG2dTextCanvas* pTxn, int x, int y, int cl, const void* str, const void** pPos);
	void NNSi_G2dTextCanvasDrawText(const NNSG2dTextCanvas* pTxn, int x, int y, int cl, u32 flags, const void* txt);

	void NNS_G2dFontInitAuto(NNSG2dFont* pFont, void* pNftrFile);

	bool NNS_G2dGetUnpackedPaletteData(void* pNclrFile, void** ppPltData );

	static inline void NNS_G2dTextCanvasInit(NNSG2dTextCanvas* pTxn, const NNSG2dCharCanvas* pCC, const NNSG2dFont* pFont, int hSpace, int vSpace)
	{
		pTxn->pCanvas       = pCC;
		pTxn->pFont         = pFont;
		pTxn->hSpace        = hSpace;
		pTxn->vSpace        = vSpace;
	}

    u16 NNS_G2dMakeCellToOams(GXOamAttr* pDstOams, u16 numDstOam, const void* pCell, const void* pMtxSR, const void* pBaseTrans, u16 affineIndex, BOOL bDoubleAffine);
    //u16 NNS_G2dMakeCellToOams(GXOamAttr* pDstOams, u16 numDstOam, const NNSG2dCellData* pCell, const MtxFx22* pMtxSR, const NNSG2dFVec2* pBaseTrans, u16 affineIndex, BOOL bDoubleAffine);

    const void* NNS_G2dGetCellDataByIdx(const void* pCellData, u16 idx);	
    //const NNSG2dCellData* NNS_G2dGetCellDataByIdx(const NNSG2dCellDataBank* pCellData, u16 idx);	
#ifdef __cplusplus
}
#endif

#endif