#include <nds.h>

#ifndef _NNS_G3D_H
#define _NNS_G3D_H

typedef struct NNSG3dResDataBlockHeader_
{
    union
    {
        u32 kind;
        char chr[4];
    };
    u32 size;
}
NNSG3dResDataBlockHeader;

typedef struct NNSG3dResDictTreeNode_
{
    u8  refBit;
    u8  idxLeft;
    u8  idxRight;
    u8  idxEntry;
}
NNSG3dResDictTreeNode;

typedef struct NNSG3dResDict_
{
    u8  revision;      // revision of dictionary
    u8  numEntry;      // num of entry(not that of tree node)
    u16 sizeDictBlk;   // size of this block
    u16 dummy_;
    u16 ofsEntry;      // (NNSG3dResDictEntryHeader*)   ((u8*)this + idxEntry)

    // patricia tree and its data entry
    NNSG3dResDictTreeNode node[1];  // variable size
}
NNSG3dResDict;

#define NNS_G3D_RESNAME_SIZE    (16)
#define NNS_G3D_RESNAME_VALSIZE (NNS_G3D_RESNAME_SIZE / sizeof(u32))

typedef union NNSG3dResName_
{
    char name[NNS_G3D_RESNAME_SIZE];
    u32  val[NNS_G3D_RESNAME_VALSIZE];
}
NNSG3dResName;

typedef struct NNSG3dResDictEntryHeader_
{
    u16 sizeUnit;      // The size (in bytes) of one data  entry
    u16 ofsName;       // 
    u8  data[4];       // The NNSG3dResDictEntryItem list (The size of each is sizeUnit)
}
NNSG3dResDictEntryHeader;

typedef enum
{
    NNS_G3D_TEXIMAGE_PARAM_TEX_ADDR_MASK  = 0x0000ffff,
    NNS_G3D_TEXIMAGE_PARAM_S_SIZE_MASK    = 0x00700000,
    NNS_G3D_TEXIMAGE_PARAM_T_SIZE_MASK    = 0x03800000,
    NNS_G3D_TEXIMAGE_PARAM_TEXFMT_MASK    = 0x1c000000,
    NNS_G3D_TEXIMAGE_PARAM_TR_MASK        = 0x20000000,

    NNS_G3D_TEXIMAGE_PARAM_TEX_ADDR_SHIFT = 0,
    NNS_G3D_TEXIMAGE_PARAM_S_SIZE_SHIFT   = 20,
    NNS_G3D_TEXIMAGE_PARAM_T_SIZE_SHIFT   = 23,
    NNS_G3D_TEXIMAGE_PARAM_TEXFMT_SHIFT   = 26,
    NNS_G3D_TEXIMAGE_PARAM_TR_SHIFT       = 29
}
NNSG3dTexImageParam;

typedef enum
{
    NNS_G3D_TEXIMAGE_PARAMEX_ORIGW_MASK  = 0x000007ff,
    NNS_G3D_TEXIMAGE_PARAMEX_ORIGH_MASK  = 0x003ff800,
    NNS_G3D_TEXIMAGE_PARAMEX_WHSAME_MASK = 0x80000000,

    NNS_G3D_TEXIMAGE_PARAMEX_ORIGW_SHIFT  = 0,
    NNS_G3D_TEXIMAGE_PARAMEX_ORIGH_SHIFT  = 11,
    NNS_G3D_TEXIMAGE_PARAMEX_WHSAME_SHIFT = 31
}
NNSG3dTexImageParamEx;

typedef struct NNSG3dResDictTexData_
{
    // 31 30 29     28  26 25 23 22 20 19   16 15                       0
    //       Pltt0  TexFmt TSize SSize         The offset that was 3 bit right-shifted
    u32           texImageParam;  // NNSG3dTexImageParam

    // 31      30           22 21     11 10     0
    // SameBit                   OrigH     OrigW
    u32           extraParam;     // NNSG3dTexImageParamEx
}
NNSG3dResDictTexData;

typedef enum
{
    NNS_G3D_RESTEX_LOADED   = 0x0001
}
NNSG3dResTexFlag;

typedef struct NNSG3dResTexInfo_
{
    u32 vramKey;         // Stores the VRAM key of the gfd library specifications
    u16 sizeTex;         // A value equivalent to the texture data after a 3-bit right-shift is entered anew
    u16 ofsDict;
    u16 flag;            // NNSG3dResTexFlag
    u16 dummy_;
    u32 ofsTex;          // The offset to the texture data (The offset from &NNSG3dResTexImage)
}
NNSG3dResTexInfo;

typedef enum
{
    NNS_G3D_RESTEX4x4_LOADED   = 0x0001   // Whether or not it is loaded into VRAM
}
NNSG3dResTex4x4Flag;

typedef struct NNSG3dResTex4x4Info_
{
    u32 vramKey;         // Stores the VRAM key of the gfd library specifications
    u16 sizeTex;         // The value when then the texture data is 3-bit right-shifted (2-bit for the palette index)
    u16 ofsDict;
    u16 flag;            // The 0 bit is the flag for whether or not the texture is loaded into VRAM
    u16 dummy_;
    u32 ofsTex;          // The offset to the texture data (The bytes from &NNSG3dResTexImage)
    u32 ofsTexPlttIdx;   // The offset to the texture palette index data (The bytes from &NNSG3dResTexImage)
}
NNSG3dResTex4x4Info;

typedef struct NNSG3dResDictPlttData_
{
    u16 offset;          // That which 3-bit right-shifted the offset towards the origin of the palette data.
    u16 flag;            // A flag that determines whether or not the 0 bit is the Tex4Pltt
}
NNSG3dResDictPlttData;

typedef enum
{
    NNS_G3D_RESPLTT_LOADED   = 0x0001,   // Whether or not it is loaded into VRAM
    NNS_G3D_RESPLTT_USEPLTT4 = 0x8000    // Whether or not the 4-color palette is in the block
}
NNSG3dResPlttFlag;

typedef struct NNSG3dResPlttInfo_
{
    u32 vramKey;      // Stores the VRAM key of the gfd library specifications
    u16 sizePltt;     // Takes what 3-bit right-shifted the data size of the texture palette.
    u16 flag;         // NNSG3dResPlttFlag
    u16 ofsDict;
    u16 dummy_;
    u32 ofsPlttData;
}
NNSG3dResPlttInfo;

typedef struct NNSG3dResTex_
{
    NNSG3dResDataBlockHeader header;
    NNSG3dResTexInfo         texInfo;
    NNSG3dResTex4x4Info      tex4x4Info;
    NNSG3dResPlttInfo        plttInfo;
    NNSG3dResDict            dict;       // The dictionary for the texture
    // The dictionary for the palette:
    // texture data (anything other than 4x4)
    // texture data (4x4)
    // texture palette index data
    // texture palette data
    //  
}
NNSG3dResTex;

typedef struct NNSG3dResDictMatData_
{
    u32           offset;       // &The offset from NNSG3dResMat data
}
NNSG3dResDictMatData;

typedef struct NNSG3dResMat_
{
    u16              ofsDictTexToMatList;  // The offset to the texture -> material ID string dictionary
    u16              ofsDictPlttToMatList; // The offset to the palette -> material ID string dictionary
    NNSG3dResDict    dict;                 // The dictionary to the NNSG3dResDictMatData
//  NNSG3dResDict    dictTexToMatList;     // (u8*)this + idxDictTexToMatList
//  NNSG3dResDict    dictPlttToMatList;    // (u8*)this + idxDictPlttToMatList
//  u8[]             matIdxData;           // (u8*)this + idxMatIdxData
//  seq of NNSG3dResMatData matData        // (u8*)this + idxMatData
}
NNSG3dResMat;

typedef struct NNSG3dResMdlInfo_
{
    u8         sbcType;       // Structure Byte Code type identifier (NNSG3dSbcType)
    u8         scalingRule;   // scaling rule identifier (NNSG3dScalingRule)
    u8         texMtxMode;    // The texture matrix calculation formula identifier (NNSG3dTexMtxMode)
    u8         numNode;

    u8         numMat;
    u8         numShp;
    u8         firstUnusedMtxStackID;
    u8         dummy_;

    // The scale value to multiply by the vertex position coordinates and its reciprocal.
    // When anything other than FX32_ONE, the scale command of the geometry engine is used to
    // process the vertex position coordinates. When the matrix is restored (using the envelope) inside the display list of the shape,
    // scaling is performed by posScale after restoration.
    fx32       posScale;      
    fx32       invPosScale;

    u16        numVertex;    // The value of the IMD file <output_info>::vertex_size
    u16        numPolygon;   // The value of the IMD file <output_info>::polygon_size
    u16        numTriangle;  // The value of the IMD file <output_info>::triangle_size
    u16        numQuad;      // The value of the IMD file <output_info>::quad_size

    fx16       boxX, boxY, boxZ; // These become the actual coordinates if multiplied by posScale
    fx16       boxW, boxH, boxD; // These become the actual coordinates if multiplied by posScale

    fx32       boxPosScale;
    fx32       boxInvPosScale;
}
NNSG3dResMdlInfo;

typedef struct NNSG3dResNodeInfo_
{
    NNSG3dResDict  dict;
    // a seq of NNSG3dResNodeSRTInfo
}
NNSG3dResNodeInfo;

typedef struct NNSG3dResMdl_
{
    u32                size;        // The size of the model
    u32                ofsSbc;      // The offset of the SBC that used &NNSG3dResMdl as the origin
    u32                ofsMat;      // The offset of the NNSG3dResMat that used &NNSG3dResMdl as the origin
    u32                ofsShp;      // The offset of the NNSG3dResShp that used &NNSG3dResMdl as the origin
    u32                ofsEvpMtx;   // The offset of NNSG3dEvpMtx that uses &NNSG3dResMdl as the origin
    NNSG3dResMdlInfo   info;        // The model information (Fixed length)
    NNSG3dResNodeInfo  nodeInfo;    // The node information
    // SBC
    // Materials
    // Shapes
    // EvpMtxs
}
NNSG3dResMdl;


#ifdef __cplusplus
extern "C"
{
#endif

	void NNS_G3dGeFlushBuffer();
	void NNS_G3dGeBufferOP_N(u32 op, const u32* args, u32 num);
	void* NNS_G3dGetResDataByName(const NNSG3dResDict* dict, const NNSG3dResName* name);

	static inline NNSG3dResMat* NNS_G3dGetMat(const NNSG3dResMdl* mdl)
	{
		return (NNSG3dResMat*)((u8*)mdl + mdl->ofsMat);
	}

	static inline NNSG3dResDictTexData* NNS_G3dGetTexDataByName(const NNSG3dResTex* tex, const NNSG3dResName* name)
	{
		return (NNSG3dResDictTexData*)NNS_G3dGetResDataByName(&tex->dict, name);
	}

	static inline NNSG3dResDictPlttData* NNS_G3dGetPlttDataByName(const NNSG3dResTex* tex, const NNSG3dResName* name)
	{
		NNSG3dResDict* dict = (NNSG3dResDict*)((u8*)tex + tex->plttInfo.ofsDict);
		return (NNSG3dResDictPlttData*)NNS_G3dGetResDataByName(dict, name);
	}

	
	static inline /*NNSG3dResMatData**/void* NNS_G3dGetMatDataByName(const NNSG3dResMat* mat, const NNSG3dResName* name)
	{
		NNSG3dResDictMatData* data = (NNSG3dResDictMatData*)NNS_G3dGetResDataByName(&mat->dict, name);
		return (/*NNSG3dResMatData*/void*)((u8*)mat + data->offset);
	}

			
#ifdef __cplusplus
}
#endif

#endif