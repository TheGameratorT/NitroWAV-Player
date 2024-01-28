#include <nds.h>

#ifndef _HW_H
#define _HW_H

#define HW_CPU_CLOCK_ARM7       33513982

#define HW_BG_PLTT              0x05000000
#define HW_BG_PLTT_END          0x05000200
#define HW_BG_PLTT_SIZE         (HW_BG_PLTT_END-HW_BG_PLTT)
#define HW_OBJ_PLTT             0x05000200
#define HW_OBJ_PLTT_END         0x05000400
#define HW_OBJ_PLTT_SIZE        (HW_OBJ_PLTT_END-HW_OBJ_PLTT)
#define HW_PLTT                 HW_BG_PLTT
#define HW_PLTT_END             HW_OBJ_PLTT_END
#define HW_PLTT_SIZE            (HW_PLTT_END-HW_PLTT)
#define HW_DB_BG_PLTT           0x05000400
#define HW_DB_BG_PLTT_END       0x05000600
#define HW_DB_BG_PLTT_SIZE      (HW_DB_BG_PLTT_END-HW_DB_BG_PLTT)
#define HW_DB_OBJ_PLTT          0x05000600
#define HW_DB_OBJ_PLTT_END      0x05000800
#define HW_DB_OBJ_PLTT_SIZE     (HW_DB_OBJ_PLTT_END-HW_DB_OBJ_PLTT)
#define HW_DB_PLTT              HW_DB_BG_PLTT
#define HW_DB_PLTT_END          HW_DB_OBJ_PLTT_END
#define HW_DB_PLTT_SIZE         (HW_DB_PLTT_END-HW_DB_PLTT)
#define HW_BG_VRAM              0x06000000
#define HW_BG_VRAM_END          0x06080000
#define HW_BG_VRAM_SIZE         (HW_BG_VRAM_END-HW_BG_VRAM)
#define HW_DB_BG_VRAM           0x06200000
#define HW_DB_BG_VRAM_END       0x06220000
#define HW_DB_BG_VRAM_SIZE      (HW_DB_BG_VRAM_END-HW_DB_BG_VRAM)
#define HW_OBJ_VRAM             0x06400000
#define HW_OBJ_VRAM_END         0x06440000
#define HW_OBJ_VRAM_SIZE        (HW_OBJ_VRAM_END-HW_OBJ_VRAM)
#define HW_DB_OBJ_VRAM          0x06600000
#define HW_DB_OBJ_VRAM_END      0x06620000
#define HW_DB_OBJ_VRAM_SIZE     (HW_DB_OBJ_VRAM_END-HW_DB_OBJ_VRAM)
#define HW_LCDC_VRAM            0x06800000
#define HW_LCDC_VRAM_END        0x06898000
#define HW_LCDC_VRAM_SIZE       (HW_LCDC_VRAM_END-HW_LCDC_VRAM)
#define HW_OAM                  0x07000000
#define HW_OAM_END              0x07000400
#define HW_OAM_SIZE             (HW_OAM_END-HW_OAM)
#define HW_DB_OAM               0x07000400
#define HW_DB_OAM_END           0x07000800
#define HW_DB_OAM_SIZE          (HW_DB_OAM_END-HW_DB_OAM)

#define	OS_VRAM_BANK_ID_A		0x0001
#define	OS_VRAM_BANK_ID_B		0x0002
#define	OS_VRAM_BANK_ID_C		0x0004
#define	OS_VRAM_BANK_ID_D		0x0008
#define	OS_VRAM_BANK_ID_E		0x0010
#define	OS_VRAM_BANK_ID_F		0x0020
#define	OS_VRAM_BANK_ID_G		0x0040
#define	OS_VRAM_BANK_ID_H		0x0080
#define	OS_VRAM_BANK_ID_I		0x0100
#define	OS_VRAM_BANK_ID_ALL		0x01ff

#define	OS_VRAM_BANK_KINDS		9


#define PAD_BUTTON_A            0x0001 // A
#define PAD_BUTTON_B            0x0002 // B
#define PAD_BUTTON_SELECT       0x0004 // SELECT
#define PAD_BUTTON_START        0x0008 // START
#define PAD_KEY_RIGHT           0x0010 // +Control Pad Right
#define PAD_KEY_LEFT            0x0020 // +Control Pad Left
#define PAD_KEY_UP              0x0040 // +Control Pad Up
#define PAD_KEY_DOWN            0x0080 // +Control Pad Down
#define PAD_BUTTON_R            0x0100 // R
#define PAD_BUTTON_L            0x0200 // L
#define PAD_BUTTON_X            0x0400 // X
#define PAD_BUTTON_Y            0x0800 // Y
#define PAD_BUTTON_DEBUG        0x2000 // Debug button

#endif