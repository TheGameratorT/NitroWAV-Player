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
|  NWAV player integration into Mario Kart DS.                |
|  This is where the wav player is installed in the game.     |
\============================================================*/

#include "NWAVPlayer.h"

#ifdef __INTELLISENSE__
#define __attribute__(x)
#endif

/*=============================================================\
|  NWAV player integration into MKDS.                          |
|  This is where the wav player is installed in the game.      |
\=============================================================*/

void NWAV_MainEventHandler(int eventID)
{
	switch (eventID)
	{
	case 0:
		break;
	default:
		break;
	}
}

/*=============================================================\
|  MKDS playback control interface replacement.                |
|  This is where the Nitro WAV player is controlled.           |
\=============================================================*/

//#define NWAV_HAS_HQ_TEMPO
//#define NWAV_HAS_HQ_PITCH

#define NWAV_FIRST_ID 611
#define NWAV_TABLE_SIZE 76
#define NWAV_HQ_TEMPO_OFFSET NWAV_TABLE_SIZE
#ifdef NWAV_HAS_HQ_TEMPO
#define NWAV_HQ_PITCH_OFFSET (NWAV_HQ_TEMPO_OFFSET * 2)
#else
#define NWAV_HQ_PITCH_OFFSET NWAV_TABLE_SIZE
#endif

static int curWav = 0;
static int pitchReturnVar = 0;
#ifdef NWAV_HAS_HQ_PITCH
static bool fastTempoMode = false;
#endif
static bool isPitchTrackPlaying = false;

//Backup the original functions
__attribute__((naked)) static void SEQ_Play(int seqID, int volume, void* handle) { asm("STMFD SP!, {R4-R6,LR}\nB 0x0210D7E0"); }

static bool GetIfSequenced(int wavID)
{
	FSFile file;
	FS_InitFile(&file);
	if (FS_OpenFileFast(&file, (void*)0x216F36C, wavID))
	{
		int magic;
		int readSize = FS_ReadFile(&file, &magic, 4);
		if (readSize)
		{
			if (magic == NWAV)
			{
				FS_CloseFile(&file);
				return false;
			}
		}
		FS_CloseFile(&file);
	}
	return true;
}

void NWAVh_Play(int seqID, int volume, void* handle)
{
	int wavID = NWAV_FIRST_ID + seqID;

	if (GetIfSequenced(wavID))
	{
		NWAV_Stop(0);
		SEQ_Play(seqID, volume, handle);
	}
	else
	{
		NNS_SndPlayerStopSeq(handle, 0);
		NWAV_Play(wavID, 0x1000, 127, 0);

#ifdef NWAV_HAS_HQ_PITCH
		pitchReturnVar = wavID;
		fastTempoMode = false;
#else
		pitchReturnVar = 0x1000;
#endif
		isPitchTrackPlaying = false;

		curWav = wavID;
	}
}

void NWAVh_Stop(void* handle, int fadeFrame)
{
	NNS_SndPlayerStopSeq(handle, fadeFrame); //Keep SSEQ support
	NWAV_Stop(fadeFrame);
}

void NWAVh_MoveVolume(void* handle, int targetVolume, int frames)
{
	NNS_SndPlayerMoveVolume(handle, targetVolume, frames); //Keep SSEQ support
	NWAV_SetVolume(targetVolume, frames);
}

void NWAVh_SetTempo(void* handle, int ratio)
{
	NNS_SndPlayerSetTempoRatio(handle, ratio); //Keep SSEQ support

#ifdef NWAV_HAS_HQ_TEMPO
	if (ratio == 300)
	{
		int wavID = curWav + NWAV_HQ_TEMPO_OFFSET;
		if (!isPitchTrackPlaying)
			NWAV_Play(wavID, 0x1000, 127, 0);
		pitchReturnVar = wavID;
		fastTempoMode = true;
	}
	else
	{
		if (!isPitchTrackPlaying)
			NWAV_Play(curWav, 0x1000, 127, 0);
		pitchReturnVar = curWav;
		fastTempoMode = false;
	}
#else
	int new_ratio = ratio << 4;
	if (!isPitchTrackPlaying)
		NWAV_SetSpeed(new_ratio);
	pitchReturnVar = new_ratio;
#endif
}

void NWAVh_SetPitch(void* handle, u16 trackBitMask, int pitch)
{
	NNS_SndPlayerSetTrackPitch(handle, trackBitMask, pitch); //Keep SSEQ support

	//Pitch Value 0 = Normal
	//Pitch Value 64 = Final Lap
	if (!isPitchTrackPlaying)
	{
		if (pitch != 0 && pitch != 64)
		{
#ifdef NWAV_HAS_HQ_PITCH
			int wavID = curWav + NWAV_HQ_PITCH_OFFSET;
			int resume = fastTempoMode ? 0x12C0/*((300 / 256) * 0x1000)*/ : 0x1000;
			NWAV_Play(wavID, 0x1000, 127, resume);
#else
			NWAV_SetPitchWaving(true, 0);
#endif
			isPitchTrackPlaying = true;
		}
	}
	else
	{
		if (pitch == 0 || pitch == 64)
		{
#ifdef NWAV_HAS_HQ_PITCH
			int resume = fastTempoMode ? 0xD40/*(0x1000 - (((300 / 256) * 0x1000) - 0x1000))*/ : 0x1000;
			NWAV_Play(pitchReturnVar, 0x1000, 127, resume);
#else
			NWAV_SetPitchWaving(false, pitchReturnVar);
#endif
			isPitchTrackPlaying = false;
		}
	}
}

// To do: Prevent loading SSEQs that are replaced by NWAVs.
