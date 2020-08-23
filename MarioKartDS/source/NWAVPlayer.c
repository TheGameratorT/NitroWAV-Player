/*============================================================\
|  This file was made by TheGameratorT.                       |
|                                                             |
|  You may modify this file and use it for whatever you want  |
|  just be sure to credit me (TheGameratorT).                 |
|                                                             |
|  Hope you like it just as much as I suffered coding this!   |
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

//#define NWAV_DEBUG_PRINT
#ifdef NWAV_DEBUG_PRINT
#include "nocashPrint.h"
#endif

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
	u8 reserved[2];

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
	.isPlaying = FALSE,
	.isPaused = TRUE,
	.speed = 0x1000,
	.volume = 127
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

static void seek(NWAVStream* strm, int pos, BOOL sample)
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

static void readSnd(NWAVStream* strm, void* dst, s32 len)
{
	if (strm->volume == 0)
		FS_SeekFile(&strm->file, len, FS_SEEK_CUR);
	else
		FS_ReadFile(&strm->file, dst, len);
}

static void prepareBuffer(NWAVStream* strm)
{
	strm->bufPage = 0;
	update(strm);
	update(strm);
}

BOOL NWAV_GetPaused() { return strm.isPaused; }
void NWAV_SetPaused(BOOL paused)
{
#ifdef NWAV_DEBUG_PRINT
	nocashPrint1("NWAV_SetPaused(%r0%)\n", paused);
#endif

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

BOOL NWAV_GetPlaying() { return strm.isPlaying; }

int NWAV_GetVolume() { return strm.volume; }
void NWAV_SetVolume(int volume, int frames)
{
#ifdef NWAV_DEBUG_PRINT
	nocashPrint2("NWAV_SetVolume(%r0%, %r1%)\n", volume, frames);
#endif

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

int NWAV_GetSampleRate() { return strm.header.sampleRate; }
int NWAV_GetCursorPos() { return strm.musicCursor; }

//For internal use
static void stop(NWAVStream* strm, int frames, BOOL waitForUpdate)
{
#ifdef NWAV_DEBUG_PRINT
	nocashPrint3("NWAV_StopInternal(%r0%, %r1%, %r2%)\n", (int)strm, frames, waitForUpdate);
#endif

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
		NWAV_SetPaused(TRUE);
		strm->isPlaying = FALSE;

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
	stop(&strm, frames, FALSE);
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

//Updates music settings each frame.
BOOL NWAV_UpdateFade()
{
	if (strm.isPlaying && strm.fadeFrame)
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
				stop(&strm, 0, FALSE);
				return FALSE;
			}
		}
	}
	return TRUE;
}

static void updateCheckEnd(NWAVStream* strm, pStrmBufT pBuf, int len)
{
	int leftOver = STRM_BUF_PAGESIZE - len;
	if (strm->loops)
	{
		if (strm->musicCursor > strm->header.loopEnd)
		{
			seek(strm, strm->header.loopStart, TRUE);
			if (leftOver > 0)
			{
				for (int i = 0; i < strm->chCount; i++)
					readSnd(strm, &(*pBuf)[i][len], leftOver);
			}
			seek(strm, strm->header.loopStart + (leftOver / strm->bytesPerSample), TRUE);
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
			stop(strm, 0, TRUE);
		}
	}
}

static void update(NWAVStream* strm)
{
	//Check for delayed stop.
	if (strm->stopMode == 2)
	{
		stop(strm, 0, FALSE);
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
		readSnd(strm, (*pBuf)[ch], len);

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
		BOOL left = i == 0;
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
	BOOL notPaused = !strm->isPaused;

	if (notPaused)
		NWAV_SetPaused(TRUE);

	setup(strm);

	if (notPaused)
		NWAV_SetPaused(FALSE);
}

fx32 NWAV_GetSpeed() { return strm.speed; }
void NWAV_SetSpeed(fx32 speed)
{
#ifdef NWAV_DEBUG_PRINT
	nocashPrint1("NWAV_SetSpeed(%r0%)\n", speed);
#endif

	fx32 sampleRate = strm.header.sampleRate << FX32_SHIFT;
	strm.playRate = FX_Mul(sampleRate, speed) >> FX32_SHIFT;
	strm.speed = speed;

	updateStreamInfo(&strm);
}

static void loadEvents(NWAVStream* strm)
{
	events = (NWAVEventInfo*)calloc(strm->header.numEvents, sizeof(NWAVEventInfo));
	for (int i = 0; i < strm->header.numEvents; i++)
	{
		int val = 0;
		FS_ReadFile(&strm->file, &val, 1);
		events[i].eventID = val;
	}
	seek(strm, headerSize + strm->eventIDBlockSize, FALSE);
	for (int i = 0; i < strm->header.numEvents; i++)
	{
		int val;
		FS_ReadFile(&strm->file, &val, 4);
		events[i].sample = val;
	}
}

void NWAV_Play(int fileID, fx32 speed, int volume, fx32 resume, int offset)
{
#ifdef NWAV_DEBUG_PRINT
	nocashPrint3("NWAV_Play(%r0%, %r1%, %r2%, ", fileID, speed, volume);
	nocashPrint2("%r0%, %r1%)\n", resume, offset);
#endif

	if (strm.isPlaying)
		stop(&strm, 0, FALSE);

	FS_InitFile(&strm.file);
	if (!FS_OpenFileFast(&strm.file, (void*)0x216F36C, fileID))
		OS_Terminate();

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

	pStrmBuf = (pStrmBufT)calloc(strm.chCount, STRM_BUF_SIZE);

	int resumePos = 0;
	if (resume)
		resumePos = FX_Mul64((s64)strm.lastPos << FX32_SHIFT, resume) >> FX32_SHIFT;

	resumePos += offset;
	resumePos %= strm.musicEnd;

	seek(&strm, resumePos, TRUE);
	NWAV_SetSpeed(strm.speed);

	strm.isPlaying = TRUE;
	strm.isPaused = TRUE;
	NWAV_SetPaused(FALSE);
}

static void StrmThread(void* arg)
{
	OSMessage message;

	while (TRUE)
	{
		OS_ReceiveMessage(&msgQ, &message, OS_MESSAGE_BLOCK);
		update((NWAVStream*)message);
	}
}

void NWAV_Init()
{
#ifdef NWAV_DEBUG_PRINT
	nocashPrint("NWAV_Init()\n");
#endif

	//Lock the channels.
	SND_LockChannel(1 << CHANNEL_L_NUM | 1 << CHANNEL_R_NUM, 0);

	//Startup stream thread.
	OS_InitMessageQueue(&msgQ, msgBuf, 1);
	OS_CreateThread(
		&strmThread,
		StrmThread,
		NULL,
		&strmThreadStack[THREAD_STACK_SIZE],
		THREAD_STACK_SIZE,
		STREAM_THREAD_PRIO
	);
	OS_WakeupThreadDirect(&strmThread);
}
