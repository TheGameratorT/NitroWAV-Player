#include <nds.h>
#include "gx.h"
#include "reg.h"
#include "types.h"

#ifndef _G3_H
#define _G3_H

#define G3OP_NOP                0x00   /* Nop            */

#define G3OP_MTX_MODE           0x10   /* MatrixMode     */
#define G3OP_MTX_PUSH           0x11   /* PushMatrix     */
#define G3OP_MTX_POP            0x12   /* PopMatrix      */
#define G3OP_MTX_STORE          0x13   /* StoreMatrix    */
#define G3OP_MTX_RESTORE        0x14   /* RestoreMatrix  */
#define G3OP_MTX_IDENTITY       0x15   /* Identity       */
#define G3OP_MTX_LOAD_4x4       0x16   /* LoadMatrix44   */
#define G3OP_MTX_LOAD_4x3       0x17   /* LoadMatrix43   */
#define G3OP_MTX_MULT_4x4       0x18   /* MultMatrix44   */
#define G3OP_MTX_MULT_4x3       0x19   /* MultMatrix43   */
#define G3OP_MTX_MULT_3x3       0x1a   /* MultMatrix33   */
#define G3OP_MTX_SCALE          0x1b   /* Scale          */
#define G3OP_MTX_TRANS          0x1c   /* Translate      */

#define G3OP_COLOR              0x20   /* Color          */
#define G3OP_NORMAL             0x21   /* Normal         */
#define G3OP_TEXCOORD           0x22   /* TexCoord       */
#define G3OP_VTX_16             0x23   /* Vertex         */
#define G3OP_VTX_10             0x24   /* VertexShort    */
#define G3OP_VTX_XY             0x25   /* VertexXY       */
#define G3OP_VTX_XZ             0x26   /* VertexXZ       */
#define G3OP_VTX_YZ             0x27   /* VertexYZ       */
#define G3OP_VTX_DIFF           0x28   /* VertexDiff     */
#define G3OP_POLYGON_ATTR       0x29   /* PolygonAttr    */
#define G3OP_TEXIMAGE_PARAM     0x2a   /* TexImageParam  */
#define G3OP_TEXPLTT_BASE       0x2b   /* TexPlttBase    */

#define G3OP_DIF_AMB            0x30   /* MaterialColor0 */
#define G3OP_SPE_EMI            0x31   /* MaterialColor1 */
#define G3OP_LIGHT_VECTOR       0x32   /* LightVector    */
#define G3OP_LIGHT_COLOR        0x33   /* LightColor     */
#define G3OP_SHININESS          0x34   /* Shininess      */

#define G3OP_BEGIN              0x40   /* Begin          */
#define G3OP_END                0x41   /* End            */

#define G3OP_SWAP_BUFFERS       0x50   /* SwapBuffers    */

#define G3OP_VIEWPORT           0x60   /* Viewport       */

#define G3OP_BOX_TEST           0x70   /* BoxTest        */
#define G3OP_POS_TEST           0x71   /* PositionTest   */
#define G3OP_VEC_TEST           0x72   /* VectorTest     */

#define G3OP_DUMMY_COMMAND      0xFF   /* Dummy command */

#define GX_PACK_POLYGONATTR_PARAM(light, polyMode, cullMode, polygonID, alpha, misc) ((u32)(((light) << 0) | ((polyMode) << 4)  | ((cullMode) << 6) | (misc) | ((polygonID) << 24) | ((alpha) << 16)))

#ifdef __cplusplus
extern "C"
{
#endif
	
	void G3i_FrustumW_(fx32 t, fx32 b, fx32 l, fx32 r, fx32 n, fx32 f, fx32 scaleW, BOOL draw, MtxFx44 *mtx);
	void G3i_OrthoW_(fx32 t, fx32 b, fx32 l, fx32 r, fx32 n, fx32 f, fx32 scaleW, BOOL draw, MtxFx44 *mtx);
	void G3X_Init();
	void G3X_InitMtxStack();
	void G3X_Reset();
	void G3X_SetClearColor(GXRgb rgb, int alpha, int depth, int polygonID, BOOL fog);

	static inline void G3_Frustum(fx32 t, fx32 b, fx32 l, fx32 r, fx32 n, fx32 f, MtxFx44 *mtx)
	{
		G3i_FrustumW_(t, b, l, r, n, f, FX32_ONE, TRUE, mtx);
	    // GX_MTXMODE_PROJECTION now
	}

	static inline void G3_Ortho(fx32 t, fx32 b, fx32 l, fx32 r, fx32 n, fx32 f, MtxFx44 *mtx)
	{
		G3i_OrthoW_(t, b, l, r, n, f, FX32_ONE, TRUE, mtx);
		// GX_MTXMODE_PROJECTION now
	}

	static inline void G3_OrthoW(fx32 t, fx32 b, fx32 l, fx32 r, fx32 n, fx32 f, fx32 scaleW, MtxFx44 *mtx)
	{
		G3i_OrthoW_(t, b, l, r, n, f, scaleW, TRUE, mtx);
		// GX_MTXMODE_PROJECTION now
	}

	static inline void G3_SwapBuffers(GXSortMode am, GXBufferMode zw)
	{
		SWAP_BUFFERS = (u32)(am | (zw << 1));
	}

	static inline void G3_ViewPort(int x1, int y1, int x2, int y2)
	{
		VIEWPORT = (u32)(x1 | (y1 << 8) | (x2 << 16) | (y2 << 24));
	}

	static inline void G3_PolygonAttr(int light, GXPolygonMode polyMode, GXCull cullMode, int polygonID, int alpha, int misc)
	{
		reg_G3_POLYGON_ATTR = GX_PACK_POLYGONATTR_PARAM(light, polyMode, cullMode, polygonID, alpha, misc);
	}

#ifdef __cplusplus
}
#endif

#endif