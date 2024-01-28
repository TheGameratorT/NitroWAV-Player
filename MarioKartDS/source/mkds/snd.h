#include <nds.h>
#include "hw.h"

#ifndef _SND_H
#define _SND_H

#define SND_COMMAND_NOBLOCK		0
#define SND_COMMAND_BLOCK		( 1 << 0 )
#define SND_COMMAND_IMMEDIATE	( 1 << 1 )

#define SND_TIMER_CLOCK			( HW_CPU_CLOCK_ARM7 / 2 )

typedef void (*SNDAlarmHandler)( void* arg );

typedef enum
{
    SND_CHANNEL_DATASHIFT_NONE,
    SND_CHANNEL_DATASHIFT_1BIT,
    SND_CHANNEL_DATASHIFT_2BIT,
    SND_CHANNEL_DATASHIFT_4BIT
}
SNDChannelDataShift;

typedef enum
{
    SND_CHANNEL_LOOP_MANUAL,
    SND_CHANNEL_LOOP_REPEAT,
    SND_CHANNEL_LOOP_1SHOT
}
SNDChannelLoop;

typedef enum SNDCommandID
{
    SND_COMMAND_START_SEQ,
    SND_COMMAND_STOP_SEQ,
    SND_COMMAND_PREPARE_SEQ,
    SND_COMMAND_START_PREPARED_SEQ,
    SND_COMMAND_PAUSE_SEQ,
    SND_COMMAND_SKIP_SEQ,
    SND_COMMAND_PLAYER_PARAM,
    SND_COMMAND_TRACK_PARAM,
    SND_COMMAND_MUTE_TRACK,
    SND_COMMAND_ALLOCATABLE_CHANNEL,
    SND_COMMAND_PLAYER_LOCAL_VAR,
    SND_COMMAND_PLAYER_GLOBAL_VAR,
    SND_COMMAND_START_TIMER,
    SND_COMMAND_STOP_TIMER,
    SND_COMMAND_SETUP_CHANNEL_PCM,
    SND_COMMAND_SETUP_CHANNEL_PSG,
    SND_COMMAND_SETUP_CHANNEL_NOISE,
    SND_COMMAND_SETUP_CAPTURE,
    SND_COMMAND_SETUP_ALARM,
    SND_COMMAND_CHANNEL_TIMER,
    SND_COMMAND_CHANNEL_VOLUME,
    SND_COMMAND_CHANNEL_PAN,
    SND_COMMAND_SURROUND_DECAY,
    SND_COMMAND_MASTER_VOLUME,
    SND_COMMAND_MASTER_PAN,
    SND_COMMAND_OUTPUT_SELECTOR,
    SND_COMMAND_LOCK_CHANNEL,
    SND_COMMAND_UNLOCK_CHANNEL,
    SND_COMMAND_STOP_UNLOCKED_CHANNEL,
    SND_COMMAND_SHARED_WORK,
    SND_COMMAND_INVALIDATE_SEQ,
    SND_COMMAND_INVALIDATE_BANK,
    SND_COMMAND_INVALIDATE_WAVE,
    SND_COMMAND_READ_DRIVER_INFO
}
SNDCommandID;

typedef enum
{
    SND_WAVE_FORMAT_PCM8,
    SND_WAVE_FORMAT_PCM16,
    SND_WAVE_FORMAT_ADPCM,
    SND_WAVE_FORMAT_PSG,
    SND_WAVE_FORMAT_NOISE = SND_WAVE_FORMAT_PSG
}
SNDWaveFormat;

#ifdef __cplusplus
extern "C"
{
#endif
	
	BOOL SND_FlushCommand(u32 flags);
	u32 SND_GetCurrentCommandTag();
	void SND_Init();
	void SND_LockChannel(u32 chBitMask, u32 flags);
	void SND_SetChannelVolume(u32 chBitMask, int volume, SNDChannelDataShift shift);
	void SND_SetupAlarm(int alarmNo, u32 tick, u32 period, SNDAlarmHandler handler, void* arg);
	void SND_SetupChannelPcm(int chNo, SNDWaveFormat format, const void *dataaddr, SNDChannelLoop loop, int loopStart, int loopLen, int volume, SNDChannelDataShift shift, int timer, int pan);
	void SND_StartTimer(u32 chBitMask, u32 capBitMask, u32 alarmBitMask, u32 flags);
	void SND_StopTimer(u32 chBitMask, u32 capBitMask, u32 alarmBitMask, u32 flags);
	void SND_UnlockChannel(u32 chBitMask, u32 flags);
	void SND_WaitForCommandProc(u32 tag);

#ifdef __cplusplus
}
#endif

#endif