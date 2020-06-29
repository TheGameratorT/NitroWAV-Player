/*============================================================\
|  This file was made by TheGameratorT.                       |
|                                                             |
|  This code is meant to work with the following template:    |
|  https://github.com/Overblade/NSMB-ASMReference             |
|                                                             |
|  You may modify this file and use it for whatever you want  |
|  just be sure to credit me (TheGameratorT).                 |
|                                                             |
|  Hope you like it just as much as I had fun coding this!    |
|                                                             |
|  ---------------------------------------------------------  |
|                                                             |
|  NWAV player integration into NSMB DS.                      |
|  This is where the wav player is installed in the game.     |
\============================================================*/

#include "NWAVPlayer.h"

#include "nitro_if.h"
#include "Extras.h"

#ifdef __INTELLISENSE__
#define __attribute__(x)
#endif // __INTELLISENSE__

static bool& doBahp = *(bool*)0x02088B9C;

//Implement enemy dance animation events
static void updateBahps()
{
	if (!NWAVPlayer::getPaused())
	{
		if (doBahp)
		{
			static bool remove = false;
			if (remove)
			{
				doBahp = false;
				remove = false;
			}
			else
			{
				remove = true;
			}
		}
	}
}

static void mainEventHandler(int eventID)
{
	switch (eventID)
	{
	//Bahp event
	case 0:
		if(getPlayerCount() == 1)
			doBahp = 1;
		break;
	//Add your events here
	default:
		break;
	}
}

void repl_0204F2F0()
{
	NNS_SndInit(); //Keep replaced instruction
	NWAVPlayer::init();
	NWAVPlayer::setEventHandler(mainEventHandler);
}

void repl_0204EC74()
{
	NNS_SndMain(); //Keep replaced instruction
	NWAVPlayer::updateFade();
	updateBahps();
}

//Reduce sound heap size and init
//void repl_020125F0() { asm("MOV R0, #0xC0000"); }
//void repl_020125F8() { asm("MOV R1, #0xC0000"); }

//NSMB uses stereo by default
__attribute__((naked)) void repl_02012FC0()
{
	asm("STR R0, [R4,#0xC]"); //Keep replaced instruction
	asm("MOV R0, #2");
	asm("STR R0, [R4,#0x344]"); //Set sound mode
	asm("BX  LR");
}

//Fix HDMA
void repl_02009434() {}

/*=============================================================\
|   NSMB playback control interface replacement.               |
|   This is where the Nitro WAV player is controlled.          |
\=============================================================*/

constexpr int firstWavID = 2089;

//Backup the original functions
__attribute__((naked)) static void playSeq(int seqID, int sfxSetID) { asm("STMFD SP!, {R4,LR}\nB 0x02011E80"); }
__attribute__((naked)) static void loadSeq(int seqID) { asm("STMFD SP!, {R4,LR}\nB 0x0204F198"); }
__attribute__((naked)) static void setTempo(int ratio) { asm("STMFD SP!, {R4,LR}\nB 0x0204E530"); }
__attribute__((naked)) static void setVolume(int targetVolume, int frames) { asm("STMFD SP!, {R4,LR}\nB 0x0204DCB8"); }
__attribute__((naked)) static void pauseSeq(bool flag) { asm("STMFD SP!, {R4,LR}\nB 0x0204E480"); }
__attribute__((naked)) static void stopSeq(int fadeFrame) { asm("STMFD SP!, {R4,LR}\nB 0x0204E424"); }

static bool GetIfSequenced(int seqID)
{
	int wavID = firstWavID + seqID;

	FSFile file;
	FS_InitFile(&file);
	if (FS_OpenFileFast(&file, (void*)0x02096114, wavID))
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

//Replace play
void nsub_02011E7C(int seqID, int sfxSetID)
{
	int wavID = firstWavID + seqID;

	int& currSeq = *(int*)0x02088B64;
	if (GetIfSequenced(seqID))
	{
		if (currSeq != seqID)
			NWAVPlayer::stop(0);
		playSeq(seqID, sfxSetID);
	}
	else
	{
		if (currSeq != seqID)
			stopSeq(0);

		NWAVPlayer::play(wavID);

		currSeq = seqID;
	}
}

//Replace load
void nsub_0204F194(int seqID)
{
	if (GetIfSequenced(seqID))
		loadSeq(seqID);
}

//Replace set tempo
void nsub_0204E52C(int ratio)
{
	setTempo(ratio);
	NWAVPlayer::setSpeed(ratio << 12 >> 8);
}

//Replace set volume
void nsub_0204DCB4(int targetVolume, int frames)
{
	setVolume(targetVolume, frames);
	NWAVPlayer::setVolume(targetVolume, frames);
}

//Replace pause
void nsub_0204E47C(bool flag)
{
	pauseSeq(flag);
	NWAVPlayer::setPaused(flag);
}

//Replace stop
void nsub_0204E420(int fadeFrame)
{
	stopSeq(fadeFrame);
	NWAVPlayer::stop(fadeFrame);
}
