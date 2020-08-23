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
|  NWAV player integration into Mario Kart DS.                |
|  This is where the wav player is installed in the game.     |
\============================================================*/

#include "NWAVPlayer.h"

#define NWAV_HAS_HQ_TEMPO 1

#define NWAV_FIRST_ID 610
#define NWAV_TABLE_SIZE 76
#define NWAV_HQ_PITCH_OFFSET NWAV_TABLE_SIZE
#define NWAV_HQ_TEMPO_OFFSET (NWAV_TABLE_SIZE * 2)

static int curFileID = 0;
static int specialSeqEndFlag = 0;
static int mutedFrames = -1;
static int mutedPos = 0;
static BOOL playingSpecial = FALSE;
static BOOL fastTempoMode = FALSE;
static BOOL isPitchTrackPlaying = FALSE;
#if NWAV_HAS_HQ_TEMPO
static BOOL hasHqTempo = FALSE;
#endif

void NWAVi_MainEventHandler(int eventID)
{
	switch (eventID)
	{
	case 0:
		break;
	default:
		break;
	}
}

void NWAVi_UpdateMute()
{
	if (mutedFrames != -1)
		mutedFrames++;
}

/*=============================================================\
|  MKDS playback control interface replacement.                |
|  This is where the Nitro WAV player is controlled.           |
\=============================================================*/

//Backup the original functions
void NWAVi_PlaySeq(int seqID, int volume, NNSSndHandle* handle);
BOOL NWAVi_LoadSeq(void* a, int seqNo, int c, int d);

static BOOL NWAVi_GetIfValidFile(int fileID)
{
	FSFile file;
	FS_InitFile(&file);
	if (FS_OpenFileFast(&file, (void*)0x216F36C, fileID))
	{
		int magic;
		int readSize = FS_ReadFile(&file, &magic, 4);
		if (readSize)
		{
			if (magic == NWAV)
			{
				FS_CloseFile(&file);
				return TRUE;
			}
		}
		FS_CloseFile(&file);
	}
	return FALSE;
}

void NWAVh_Play(int seqID, int volume, NNSSndHandle* handle, BOOL mainSeq)
{
	int fileID = NWAV_FIRST_ID + seqID;
	if (NWAVi_GetIfValidFile(fileID))
	{
		NNS_SndPlayerStopSeq(handle, 0);
		NWAV_Play(fileID, 0x1000, volume, 0, 0);

		if (mainSeq)
		{
#if NWAV_HAS_HQ_TEMPO
			hasHqTempo = NWAVi_GetIfValidFile(fileID + NWAV_HQ_TEMPO_OFFSET);
			specialSeqEndFlag = hasHqTempo ? fileID : 0x1000;
#else
			specialSeqEndFlag = 0x1000;
#endif
			playingSpecial = FALSE;
			fastTempoMode = FALSE;
			isPitchTrackPlaying = FALSE;

			curFileID = fileID;
		}
		else
		{
			playingSpecial = TRUE;
		}
	}
	else
	{
		NWAV_Stop(0);
		NWAVi_PlaySeq(seqID, volume, handle);

		if (mainSeq)
			curFileID = 0;
	}
}

void NWAVh_SetMute(NNSSndHandle* handle, u16 trackBitMask, BOOL flag)
{
	NNS_SndPlayerSetTrackMute(handle, trackBitMask, flag);

	if (flag && NWAV_GetPlaying())
	{
		mutedFrames = 0;
		mutedPos = NWAV_GetCursorPos();
		NWAV_Stop(0);
	}
	else if (!flag)
	{
		if (curFileID != 0)
		{
			int offset = mutedPos + (NWAV_GetSampleRate() * (mutedFrames / 60));
			mutedFrames = -1;
#if NWAV_HAS_HQ_TEMPO
			if (hasHqTempo)
			{
				NWAV_Play(specialSeqEndFlag, 0, 0, 0, offset);
			}
			else
			{
#endif
				NWAV_Play(curFileID, specialSeqEndFlag, 0, 0, offset);
#if NWAV_HAS_HQ_TEMPO
			}
#endif
		}
		else
		{
			NWAV_Stop(0);
		}

		playingSpecial = FALSE;
	}
}

void NWAVh_Stop(NNSSndHandle* handle, int fadeFrame)
{
	NNS_SndPlayerStopSeq(handle, fadeFrame); //Keep SSEQ support
	NWAV_Stop(fadeFrame);
}

void NWAVh_MoveVolume(NNSSndHandle* handle, int targetVolume, int frames)
{
	NNS_SndPlayerMoveVolume(handle, targetVolume, frames); //Keep SSEQ support
	NWAV_SetVolume(targetVolume, frames);
}

void NWAVh_SetVolume(NNSSndHandle* handle, int volume)
{
	NNS_SndPlayerSetVolume(handle, volume); //Keep SSEQ support
	NWAV_SetVolume(volume, 0);

}

void NWAVh_SetTempo(NNSSndHandle* handle, int ratio)
{
	NNS_SndPlayerSetTempoRatio(handle, ratio); //Keep SSEQ support

	if (curFileID == 0) //If main track is not NWAV
		return;

#if NWAV_HAS_HQ_TEMPO
	if (hasHqTempo)
	{
		//Ratio Value 256 = Normal
		//Ratio Value 300 = Final Lap
		if (ratio == 300)
		{
			int fileID = curFileID + NWAV_HQ_TEMPO_OFFSET;
			if (!isPitchTrackPlaying)
				NWAV_Play(fileID, 0x1000, 0, 0, 0);
			specialSeqEndFlag = fileID;
			fastTempoMode = TRUE;
		}
		else
		{
			if (!isPitchTrackPlaying)
				NWAV_Play(curFileID, 0x1000, 0, 0, 0);
			specialSeqEndFlag = curFileID;
			fastTempoMode = FALSE;
		}
	}
	else
	{
#endif
		int new_ratio = ratio << 4;
		if (!isPitchTrackPlaying)
			NWAV_SetSpeed(new_ratio);
		specialSeqEndFlag = new_ratio;
#if NWAV_HAS_HQ_TEMPO
	}
#endif
}

void NWAVh_SetPitch(NNSSndHandle* handle, u16 trackBitMask, int pitch)
{
	NNS_SndPlayerSetTrackPitch(handle, trackBitMask, pitch); //Keep SSEQ support

	if (curFileID == 0) //If main track is not NWAV
		return;

	//Pitch Value 0 = Normal
	//Pitch Value 64 = Final Lap
	if (!isPitchTrackPlaying)
	{
		if (pitch != 0 && pitch != 64)
		{
			int fileID = curFileID + NWAV_HQ_PITCH_OFFSET;
			int resume = fastTempoMode ? 0x12C0/*((300 / 256) * 0x1000)*/ : 0x1000;
			NWAV_Play(fileID, 0x1000, 0, resume, 0);
			isPitchTrackPlaying = TRUE;
		}
	}
	else
	{
		if (pitch == 0 || pitch == 64)
		{
			if (!playingSpecial)
			{
				int resume = fastTempoMode ? 0xD40/*(0x1000 - (((300 / 256) * 0x1000) - 0x1000))*/ : 0x1000;
#if NWAV_HAS_HQ_TEMPO
				if (hasHqTempo)
				{
					NWAV_Play(specialSeqEndFlag, 0x1000, 0, resume, 0);
				}
				else
				{
#endif
					NWAV_Play(curFileID, specialSeqEndFlag, 0, resume, 0);
#if NWAV_HAS_HQ_TEMPO
				}
#endif
			}

			isPitchTrackPlaying = FALSE;
		}
	}
}

BOOL NWAVh_LoadSeq(void* a, int seqNo, int c, int d)
{
	int fileID = NWAV_FIRST_ID + seqNo;
	if (NWAVi_GetIfValidFile(fileID))
		return FALSE;

	return NWAVi_LoadSeq(a, seqNo, c, d); //Keep SSEQ support
}

// To do:
//  - Check issue with multiplayer menu.
//  - Fix small loop gap. (in all NWAV variants)
