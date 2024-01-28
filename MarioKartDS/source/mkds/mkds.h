#include <nds.h>

#ifndef _MKDS2_H
#define _MKDS2_H

typedef enum
{
    MKDS_BATTLE_MODE_BALLOON_BATTLE = 0,
    MKDS_BATTLE_MODE_SHINE_RUNNERS = 1
}
MKDSBattleMode;

typedef enum
{
    MKDS_CC_MODE_50CC = 0,
    MKDS_CC_MODE_100CC = 1,
	MKDS_CC_MODE_150CC = 2
}
MKDSCCMode;

typedef enum
{
    MKDS_COURSE_MODE_CHOOSE = 0,
    MKDS_COURSE_MODE_IN_ORDER = 1,
	MKDS_COURSE_MODE_RANDOM = 2
}
MKDSCourseMode;

typedef enum
{
    MKDS_CPU_MODE_NONE = 0,//?
    MKDS_CPU_MODE_EASY = 1,
	MKDS_CPU_MODE_NORMAL = 2,
	MKDS_CPU_MODE_HARD = 3
}
MKDSCPUMode;

typedef enum
{
    MKDS_MIRROR_MODE_OFF = 0,
    MKDS_MIRROR_MODE_ON = 1
}
MKDSMirrorMode;

typedef enum
{
    MKDS_RACE_MODE_GRAND_PRIX = 0,
    MKDS_RACE_MODE_TIME_TRAILS = 1,
	MKDS_RACE_MODE_BATTLE = 2,
	MKDS_RACE_MODE_VS = 3,
	MKDS_RACE_MODE_MISSION_RUN = 4
	//MKDS_RACE_MODE_WIFI = 5 //?
}
MKDSRaceMode;

typedef enum
{
    MKDS_RULES_FREE = 0,
    MKDS_RULES_NR_WINS = 1,
	MKDS_RULES_NR_RACES = 2
}
MKDSRules;

typedef enum
{
    MKDS_TEAM_MODE_OFF = 0,
    MKDS_TEAM_MODE_ON = 1
}
MKDSTeamMode;

typedef enum
{
    MKDS_DISPLAY_MODE_DEFAULT = 0,
	MKDS_DISPLAY_MODE_TITLE_MENU = 1,
	MKDS_DISPLAY_MODE_REPLAY = 2,
	MKDS_DISPLAY_MODE_COURSE_INTRO = 3,
	MKDS_DISPLAY_MODE_BATTLE_INTRO = 4,
	MKDS_DISPLAY_MODE_AWARD_CEREMONY = 5,
	MKDS_DISPLAY_MODE_CREDITS = 6
}
MKDSDisplayMode;

typedef struct
{
    u32 unk0;
    u32 unk1;
    u32 unk2;
    u32 laptimes;
    u32 currentLap;
    u32 unk3;
    u32 totalMillis;
    u16 rageFlags;
    u16 currentCpoi;
    u16 lastCorrectCpoi;
    u16 currentKeyCpoi;
    u16 currentCpat;
    u16 highestReachedLap;
    u16 unk4;
    u16 unk5;
    u32 unk6;
    u32 unk7;
    u32 unk8;
    u32 cpoiMask[16];
} race_driver_status_t;

typedef struct
{
    u32 unk0;
    u32 unk1;
    u32 laptimes;
    u16 ranktimeGP;
    u16 unk2;
    u16 unk3;
    u16 unk4;
    race_driver_status_t driverStatuses[8];
    //other shit nobody cares about (yet)
} race_status_t;

#ifdef __cplusplus
extern "C"
{
#endif

	void* MKDS_Alloc(void* something, int count);
	void* MKDS_AllocEx(void* something, int count, int alignment);
	void MKDS_Free(void* something, void* ptr);

	void* GetNNSFndHeapHandle(void* something);
	void* sub_20364E8();

	void Init3D();

	void SetNextBattleMode(MKDSBattleMode mode);
	void SetNextCCMode(MKDSCCMode mode);
	void SetNextCourse(int course);
	void SetNextCourseMode(MKDSCourseMode mode);
	void SetNextCPUMode(MKDSCPUMode mode);
	void SetNextCup(int cup);
	void SetNextDriverXConfig(int driver, int character, int kart, int unka, int unkb, int unkc);
	void SetNextMirrorMode(MKDSMirrorMode mode);
	void SetNextNrRacesNrWins(int count);
	void SetNextRaceMode(MKDSRaceMode mode);
	void SetNextRules(MKDSRules rules);
	void SetNextTeamMode(MKDSTeamMode mode);
	void SetNextTTGhost(int ghost);

	void SetNextDisplayMode(MKDSDisplayMode mode);

	void SetObjectPolygonID(void* pObjectInfo, int PolygonID);

	void* GetBBMtxAtPosition(fx32* position);

	void DrawObject2DBB(int a, void* pObjectInfo, void* pObjectRTInfo);
	void DrawObject2D3D(int a, void* pObjectInfo, void* pObjectRTInfo);

	void sub_209BBD0(int a, void* pObjectInfo);

	void Objects_SetNSBTPFrame(void* pObjectInfo, int FrameNr);
	void Objects_SetNSBCAFrame(void* pObjectInfo, int FrameNr);

	void Route_CalcCurrentPointXYZ(void* pRouteInfo, fx32* dst);
	void Route_CalcCurrentPointXZLinearY(void* pRouteInfo, fx32* dst);
	void Route_CalcCurrentPointLinearXYZ(void* pRouteInfo, fx32* dst);
	void Route_Progress(void* pRouteInfo);

	//Object Stuff
	//void* StoreObjectFunctions3D(void (*LoadFunc)(), void (*DrawFunc)(), void (*UnkFunc)());

	void* Race_GetDriverInfo(int idx);
	int Race_GetPlayerId();

	int GetDisplayMode();
	int GetCurrentMenu();
	
	void* Process_getCurrent();
	void* Process_getHeapHandle(void* process);
	
				
				
#ifdef __cplusplus
}
#endif

#endif