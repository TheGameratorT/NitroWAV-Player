/*============================================================\
|  This file was made by TheGameratorT.                       |
|                                                             |
|  You may modify this file and use it for whatever you want  |
|  just be sure to credit me (TheGameratorT).                 |
|                                                             |
|  Hope you like it just as much as I had fun coding this!    |
|                                                             |
|  ---------------------------------------------------------  |
|                                                             |
|  NWAV player core.                                          |
|  This is main code that allows for music playback on DS.    |
\============================================================*/

#include "NWAVPlayer.h"

#define CHANNEL_L_NUM 6
#define CHANNEL_R_NUM 7
#define ALARM_NUM 1
#define STREAM_THREAD_PRIO 12
#define THREAD_STACK_SIZE 1024
#define STRM_BUF_PAGESIZE (64 * 32)
#define STRM_BUF_PAGECOUNT 2
#define STRM_BUF_SIZE (STRM_BUF_PAGESIZE * STRM_BUF_PAGECOUNT)

#define MATH_CLAMP(x, low, high) ( ( (x) > (high) ) ? (high) : ( ( (x) < (low) ) ? (low) : (x) ) )

typedef struct NWAVEventInfo
{
	int eventID;
	int sample;
} NWAVEventInfo;

typedef struct NWAVHeader
{
	int magic;
	int fileSize;
	int sampleRate;
	int loopStart;
	int loopEnd;
	SNDWaveFormat format : 8;
	u8 stereo;
	u8 numEvents;
	u8 padding;
} NWAVHeader;

typedef struct NWAVStream
{
	FSFile file;
	NWAVHeader header;

	u8 isPlaying;
	u8 isPaused;
	s8 pitchIndex;
	u8 reserved;

	fx32 speed;
	int playRate;
	int volume;

	int chMask;
	int bufPage;
	int musicEnd;
	int musicCursor;
	int samplesPerUpdate;
	int targetVolume;
	int fadeDec;
	int fadeFrame;
	int lastPos;
	u8 loops;
	u8 stopMode;
	u8 chCount;
	u8 bytesPerSample;

	int eventIDBlockSize;
	int eventBlockSize;

	NWAVEventHandler eventHandler;
} NWAVStream;

static NWAVStream strm = {
	.isPlaying = false,
	.isPaused = true,
	.speed = 0x1000,
	.volume = 127,
	.pitchIndex = -1
};

static const int headerSize = sizeof(NWAVHeader);

static u8 strmThreadStack[THREAD_STACK_SIZE];
static OSThread strmThread;
static OSMessageQueue msgQ;
static OSMessage msgBuf[1];

static NWAVEventInfo* events;

typedef u8(*pStrmBufT)[2][STRM_BUF_SIZE];
static pStrmBufT pStrmBuf;

static void update(NWAVStream* strm);

static void SoundAlarmHandler(void* arg)
{
	OS_SendMessage(&msgQ, (OSMessage)arg, OS_MESSAGE_NOBLOCK);
}

static int alignSample(NWAVStream* strm, int pos)
{
	if (strm->header.stereo)
	{
		int sampleAlign = strm->samplesPerUpdate;
		int unaligned = (pos % sampleAlign);

		if (unaligned > (sampleAlign / 2))
			pos += (sampleAlign - unaligned);
		else
			pos -= unaligned;
	}
	return pos;
}

static void seek(NWAVStream* strm, int pos, bool sample)
{
	if (sample)
	{
		pos = alignSample(strm, pos);
		strm->musicCursor = pos;

		pos *= strm->chCount;
		pos *= strm->bytesPerSample;
		pos += headerSize;
		pos += strm->eventBlockSize;
	}

	FS_SeekFile(&strm->file, pos, FS_SEEK_SET);
}

static void prepareBuffer(NWAVStream* strm)
{
	strm->bufPage = 0;
	update(strm);
	update(strm);
}

bool NWAV_GetPaused() { return strm.isPaused; }
void NWAV_SetPaused(bool paused)
{
	if (strm.isPlaying && strm.isPaused != paused)
	{
		if (paused)
		{
			SND_StopTimer(strm.chMask, 0, 1 << ALARM_NUM, 0);
		}
		else
		{
			prepareBuffer(&strm);
			SND_StartTimer(strm.chMask, 0, 1 << ALARM_NUM, 0);
		}
		strm.isPaused = paused;
	}
}

int NWAV_GetVolume() { return strm.volume; }
void NWAV_SetVolume(int volume, int frames)
{
	if (strm.volume == volume)
		return;

	if (frames == 0)
	{
		SND_SetChannelVolume(strm.chMask, volume, SND_CHANNEL_DATASHIFT_NONE);
		strm.volume = volume;
	}
	else
	{
		int volumeDiff = strm.volume - volume;
		int remove = volumeDiff > 0 ? 1 : -1;

		strm.fadeDec = (volumeDiff / frames) + remove;
		strm.fadeFrame = frames;
		strm.targetVolume = volume;
	}
}

//For internal use
static void stop(NWAVStream* strm, int frames, bool waitForUpdate)
{
	if (!strm->isPlaying)
		return;

	if (frames || waitForUpdate)
	{
		if (frames)
			NWAV_SetVolume(0, frames);
		strm->stopMode = waitForUpdate + 1;
	}
	else
	{
		NWAV_SetPaused(true);
		strm->isPlaying = false;

		strm->lastPos = strm->musicCursor;

		FS_CloseFile(&strm->file);
		if (strm->header.numEvents)
			free(events);
		free(pStrmBuf);
	}
}

//For external use
void NWAV_Stop(int frames)
{
	stop(&strm, frames, false);
}

void NWAV_SetEventHandler(NWAVEventHandler func) { strm.eventHandler = func; }
static void updateEvents(NWAVStream* strm)
{
	if (!strm->eventHandler)
		return;

	for (int i = 0; i < strm->header.numEvents; i++)
	{
		NWAVEventInfo info = events[i];
		if (info.sample > strm->musicCursor &&
			info.sample < strm->musicCursor + strm->samplesPerUpdate)
		{
			strm->eventHandler(info.eventID);
		}
	}
}

static int pitchForIndex[] = {
	0xF00,
	0xE00,
	0xF00,
	0x1000,
	0x1100,
	0x1200,
	0x1100,
	0x1000
};

//Sets the music as faster, but does not care to align the buffer.
//WARNING: Be extremely cautious.
static void setSpeedFast(NWAVStream* strm, fx32 speed)
{
	fx32 sampleRate = strm->header.sampleRate << FX32_SHIFT;
	int rate = FX_Mul(sampleRate, speed) >> FX32_SHIFT;

	s32 timerValue = SND_TIMER_CLOCK / rate;
	SND_SetChannelTimer(strm->chMask, timerValue);
}

//Used to switch pitch per frame update.
//WARNING: Somewhat misaligned with the buffer.
static void setAndAlternatePitch(NWAVStream* strm)
{
	setSpeedFast(strm, pitchForIndex[strm->pitchIndex]);
	strm->pitchIndex++;
	if (strm->pitchIndex > 7)
		strm->pitchIndex = 0;
}

void NWAV_SetPitchWaving(bool waving, fx32 returnSpeed)
{
	if (strm.isPlaying)
	{
		if (waving)
		{
			NWAV_SetSpeed(0x1000);
			strm.pitchIndex = 0;
		}
		else
		{
			NWAV_SetSpeed(returnSpeed);
			strm.pitchIndex = -1;
		}
	}
}

//Updates music settings each frame.
bool NWAV_UpdatePerFrame()
{
	if (strm.isPlaying)
	{
		if (strm.pitchIndex != -1)
			setAndAlternatePitch(&strm);

		if (strm.fadeFrame)
		{
			int newVolume = strm.volume - strm.fadeDec;
			newVolume = MATH_CLAMP(newVolume, 0, 127);

			NWAV_SetVolume(newVolume, 0);
			strm.fadeFrame--;

			if (strm.fadeFrame == 0)
			{
				NWAV_SetVolume(strm.targetVolume, 0);
				if (strm.stopMode == 1)
				{
					stop(&strm, 0, false);
					return false;
				}
			}
		}
	}
	return true;
}

static void updateCheckEnd(NWAVStream* strm, pStrmBufT pBuf, int len)
{
	int leftOver = STRM_BUF_PAGESIZE - len;
	if (strm->loops)
	{
		if (strm->musicCursor > strm->header.loopEnd)
		{
			seek(strm, strm->header.loopStart, true);
			if (leftOver > 0)
			{
				for (int i = 0; i < strm->chCount; i++)
					FS_ReadFile(&strm->file, &(*pBuf)[i][len], leftOver);
			}
			seek(strm, strm->header.loopStart + (leftOver / strm->bytesPerSample), true);
		}
	}
	else
	{
		if (strm->musicCursor > strm->musicEnd)
		{
			if (leftOver > 0)
			{
				for (int i = 0; i < strm->chCount; i++)
					MI_CpuClear8(&(*pBuf)[i][len], leftOver);
			}
			stop(strm, 0, true);
		}
	}
}

static void update(NWAVStream* strm)
{
	//Check for delayed stop.
	if (strm->stopMode == 2)
	{
		stop(strm, 0, false);
		return;
	}

	//Get buffer page and swap.
	pStrmBufT pBuf = (pStrmBufT)(*(*pStrmBuf) + (STRM_BUF_PAGESIZE * strm->bufPage));

	//Get read length.
	int len = STRM_BUF_PAGESIZE;
	int limit = strm->loops ? strm->header.loopEnd : strm->musicEnd;
	int remain = (limit - strm->musicCursor) * strm->bytesPerSample;
	if (remain < len)
		len = remain;

	//Read the data to the buffer.
	for (int ch = 0; ch < strm->chCount; ch++)
		FS_ReadFile(&strm->file, (*pBuf)[ch], len);

	strm->musicCursor += strm->samplesPerUpdate;

	updateCheckEnd(strm, pBuf, len);
	updateEvents(strm);

	strm->bufPage++;
	if (strm->bufPage == STRM_BUF_PAGECOUNT)
		strm->bufPage = 0;
}

static void setup(NWAVStream* strm)
{
	s32 timerValue = SND_TIMER_CLOCK / strm->playRate;
	u32 alarmPeriod = timerValue * strm->samplesPerUpdate / 32;

	for (int i = 0; i < strm->chCount; i++)
	{
		bool left = i == 0;
		SND_SetupChannelPcm(
			left ? CHANNEL_L_NUM : CHANNEL_R_NUM,
			strm->header.format,
			left ? (*pStrmBuf)[0] : (*pStrmBuf)[1],
			SND_CHANNEL_LOOP_REPEAT,
			0,
			STRM_BUF_SIZE / sizeof(u32),
			strm->volume,
			SND_CHANNEL_DATASHIFT_NONE,
			timerValue,
			!strm->header.stereo ? 64 : (left ? 0 : 127)
		);
	}
	SND_SetupAlarm(ALARM_NUM, alarmPeriod, alarmPeriod, SoundAlarmHandler, strm);
}

static void updateStreamInfo(NWAVStream* strm)
{
	bool notPaused = !strm->isPaused;

	if (notPaused)
		NWAV_SetPaused(true);

	setup(strm);

	if (notPaused)
		NWAV_SetPaused(false);
}

fx32 NWAV_GetSpeed() { return strm.speed; }
void NWAV_SetSpeed(fx32 speed)
{
	fx32 sampleRate = strm.header.sampleRate << FX32_SHIFT;
	strm.playRate = FX_Mul(sampleRate, speed) >> FX32_SHIFT;
	strm.speed = speed;

	updateStreamInfo(&strm);
}

static void loadEvents(NWAVStream* strm)
{
	events = (NWAVEventInfo*)malloc(sizeof(NWAVEventInfo) * strm->header.numEvents);
	for (int i = 0; i < strm->header.numEvents; i++)
	{
		int val = 0;
		FS_ReadFile(&strm->file, &val, 1);
		events[i].eventID = val;
	}
	seek(strm, headerSize + strm->eventIDBlockSize, false);
	for (int i = 0; i < strm->header.numEvents; i++)
	{
		int val;
		FS_ReadFile(&strm->file, &val, 4);
		events[i].sample = val;
	}
}

void NWAV_Play(int fileID, fx32 speed, int volume, fx32 resume)
{
	if (strm.isPlaying)
		stop(&strm, 0, false);

	FS_InitFile(&strm.file);
	if (!FS_OpenFileFast(&strm.file, (void*)0x216F36C, fileID))
		OS_Panic();

	FS_ReadFile(&strm.file, &strm.header, headerSize);

	//Setup play
	if (speed != 0)
		strm.speed = speed;
	if (volume != 0)
		strm.volume = volume;

	//Reset variables
	strm.loops = strm.header.loopEnd != 0;
	strm.fadeDec = 0;
	strm.fadeFrame = 0;
	strm.stopMode = 0;
	strm.chCount = strm.header.stereo ? 2 : 1;
	strm.bytesPerSample = strm.header.format ? 2 : 1;
	strm.samplesPerUpdate = (STRM_BUF_PAGESIZE / strm.bytesPerSample);
	strm.chMask = 1 << CHANNEL_L_NUM | ((1 << CHANNEL_R_NUM) * strm.header.stereo);

	if (strm.header.numEvents)
	{
		int unalignedEvents = (strm.header.numEvents % 4);
		strm.eventIDBlockSize = strm.header.numEvents + (4 - unalignedEvents);
		strm.eventBlockSize = strm.eventIDBlockSize + (strm.header.numEvents * 4);
		loadEvents(&strm);
	}
	else
	{
		strm.eventIDBlockSize = 0;
		strm.eventBlockSize = 0;
	}

	strm.musicEnd = (((strm.header.fileSize - headerSize - strm.eventBlockSize) / strm.chCount) / strm.bytesPerSample);

	pStrmBuf = (pStrmBufT)malloc(STRM_BUF_SIZE * strm.chCount);

	int resumePos = 0;
	if (resume)
		resumePos = FX_MulInline64((s64)strm.lastPos << FX32_SHIFT, resume) >> FX32_SHIFT;

	seek(&strm, resumePos, true);
	NWAV_SetSpeed(strm.speed);

	strm.isPlaying = true;
	strm.isPaused = true;
	NWAV_SetPaused(false);
}

static void StrmThread(void* arg)
{
	OSMessage message;

	while (true)
	{
		OS_ReceiveMessage(&msgQ, &message, OS_MESSAGE_BLOCK);
		update((NWAVStream*)message);
	}
}

void NWAV_Init()
{
	//Lock the channels.
	SND_LockChannel(1 << CHANNEL_L_NUM | 1 << CHANNEL_R_NUM, 0);

	//Startup stream thread.
	OS_InitMessageQueue(&msgQ, msgBuf, 1);
	OS_CreateThread(&strmThread,
		StrmThread,
		NULL,
		&strmThreadStack[THREAD_STACK_SIZE],
		THREAD_STACK_SIZE,
		STREAM_THREAD_PRIO);
	OS_WakeupThreadDirect(&strmThread);
}
