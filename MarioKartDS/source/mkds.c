#include <nds.h>
#include "mkds.h"

/*void MKDS_StartRace(MKDSRaceMode mode, int course)
{
	switch(mode)
	{
	case MKDS_RACE_MODE_GRAND_PRIX:
		return;
	case MKDS_RACE_MODE_TIME_TRAILS:
	case MKDS_RACE_MODE_BATTLE:
		return;
	case MKDS_RACE_MODE_VS:
		return;
	case MKDS_RACE_MODE_MISSION_RUN:
		return;
	}
}*/


/*void MKDS_StartTimeTrail(int course, int character, int kart)
{
	SetNextCPUMode(MKDS_CPU_MODE_NONE);
	//setup drivers here
	SetNextDriverXConfig(0, character, kart, 0, -1, 3);
	int i;
	for(i = 1; i < 8; i++)
	{
		SetNextDriverXConfig(i, -1, -1, 4, -1, 3);
	}
	//
	SetNextRaceMode(MKDS_RACE_MODE_TIME_TRAILS);
	SetNextCup(0);//tempoarly
	SetNextCourse(course);
	SetNextCCMode(MKDS_CC_MODE_150CC);//150CC for time trails
	SetNextMirrorMode(MKDS_MIRROR_MODE_OFF);
	SetNextTTGhost(4);//no ghost
	SetNextBattleMode(MKDS_BATTLE_MODE_BALLOON_BATTLE);//no effect
	SetNextTeamMode(MKDS_TEAM_MODE_OFF);
	SetNextNrRacesNrWins(0);
	SetNextRules(MKDS_RULES_FREE);
	SetNextCourseMode(MKDS_COURSE_MODE_CHOOSE);
	((vu16*)0x021653BC)[0] = 2;
}*/