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
|  NWAV player core.                                          |
|  This is main code that allows for music playback on DS.    |
\============================================================*/

#include "NWAVPlayer.h"
#include "Memory.h"

/*==============================================================\
|  Player settings, change these according to your game needs.  |
\==============================================================*/

#define CHANNEL_L_NUM 6
#define CHANNEL_R_NUM 7
#define CHANNEL_MASK (1 << CHANNEL_L_NUM | ((1 << CHANNEL_R_NUM) * hInfo.stereo))
#define ALARM_NUM 1
#define STREAM_THREAD_PRIO 12
#define THREAD_STACK_SIZE 512
#define STRM_BUF_PAGESIZE (64 * 32)
#define STRM_BUF_PAGESIZE_STEREO (STRM_BUF_PAGESIZE * 2)
#define STRM_BUF_SIZE (STRM_BUF_PAGESIZE * 2)

namespace NWAVPlayer
{
	/*========================\
	|  Typedef declarations.  |
	\========================*/

	typedef u8(*pStrmBufT)[2][STRM_BUF_SIZE];

	/*==========================\
	|  Structure declarations.  |
	\==========================*/

	struct EventInfo
	{
		int eventID;
		int sample;
	};

	struct Header
	{
		int magic;
		int fileSize;
		int sampleRate;
		int loopStart;
		int loopEnd;
		u8 format;
		u8 stereo;
		u8 numEvents;
		u8 padding;
	};

	struct StreamInfo
	{
		u8 isPlaying;
		u8 isPaused;
		u8 reserved[2];

		fx32 speed;
		int volume;

		int bufPage;
		int musicEnd;
		int musicCursor;
		int samplesPerUpdate;
		int playRate;
		int targetVolume;
		int fadeDec;
		int fadeFrame;
		u8 loops;
		u8 stopMode;
		u8 chCount;
		u8 bytesPerSample;

		int eventIDBlockSize;
		int eventBlockSize;

		EventHandler eventHandler;
	};

	const int headerSize = sizeof(Header);
	const int eventInfoSize = sizeof(EventInfo);

	/*=======================\
	|  Static declarations.  |
	\=======================*/

	static Header hInfo;
	static StreamInfo sInfo = {
		.isPlaying = false,
		.isPaused = true,
		.speed = 0x1000,
		.volume = 127,
		.fadeFrame = 0
	};
	static FSFile file;

	static u8 strmThreadStack[THREAD_STACK_SIZE];
	static OSThread strmThread;
	static OSMessageQueue msgQ;
	static OSMessage msgBuf;

	static EventInfo* events;
	static pStrmBufT pStrmBuf;

	/*=========================\
	|  Function declarations.  |
	\=========================*/

	void update(StreamInfo* sInfo);

	//Aligns a sample so it doesn't mess up on stereo.
	int alignSample(int pos)
	{
		if (hInfo.stereo)
		{
			int sampleAlign = sInfo.samplesPerUpdate;
			int unaligned = (pos % sampleAlign);

			if (unaligned > (sampleAlign / 2))
				pos += (sampleAlign - unaligned);
			else
				pos -= unaligned;
		}
		return pos;
	}

	//Goes to a certain position in the file, either based on byte index or music sample.
	void seek(int pos, bool sample)
	{
		if (sample)
		{
			//Calculate file absolute file position.
			pos = alignSample(pos);
			sInfo.musicCursor = pos;

			pos *= sInfo.chCount;
			pos *= sInfo.bytesPerSample;
			pos += headerSize;
			pos += sInfo.eventBlockSize;
		}

		//Seek position in file.
		FS_SeekFile(&file, pos, FS_SEEK_SET);
	}

	//Use this before playing or resuming music. (Keeps buffer aligned and makes it update instantly)
	void prepareBuffer()
	{
		sInfo.bufPage = 0;
		update(&sInfo);
		update(&sInfo);
	}

	//Gets if the music is paused.
	bool getPaused() { return sInfo.isPaused; }

	//Sets if the music is paused by stopping or starting the timers.
	void setPaused(bool paused)
	{
		//If music is playing and stopped flag isn't the one already set.
		if (sInfo.isPlaying && sInfo.isPaused != paused)
		{
			if (paused)
			{
				SND_StopTimer(CHANNEL_MASK, 0, 1 << ALARM_NUM, 0);
			}
			else
			{
				prepareBuffer();
				SND_StartTimer(CHANNEL_MASK, 0, 1 << ALARM_NUM, 0);
			}
			sInfo.isPaused = paused;
		}
	}

	//Gets the current volume.
	int getVolume() { return sInfo.volume; }

	//Sets the volume by shifting it during the specified frame period.
	void setVolume(int volume, int frames)
	{
		//Do nothing if volume doesn't change.
		if (sInfo.volume == volume)
			return;

		if (frames == 0)
		{
			SND_SetChannelVolume(CHANNEL_MASK, volume, SND_CHANNEL_DATASHIFT_NONE);
			sInfo.volume = volume;
		}
		else
		{
			int volumeDiff = sInfo.volume - volume;
			int remove = volumeDiff > 0 ? 1 : -1;

			sInfo.fadeDec = (volumeDiff / frames) + remove;
			sInfo.fadeFrame = frames;
			sInfo.targetVolume = volume;
		}
	}

	//Stops the music. (For internal use ONLY)
	//WARNING: You can either use frames or waitForUpdate, not both!!!
	void stop(int frames, bool waitForUpdate)
	{
		//Do nothing if not playing.
		if (!sInfo.isPlaying)
			return;

		if (frames || waitForUpdate)
		{
			//Begin special sound stop.
			if (frames)
				setVolume(0, frames);
			sInfo.stopMode = waitForUpdate + 1;
		}
		else
		{
			//Stop music timers.
			setPaused(true);
			sInfo.isPlaying = false;

			//Close the file.
			FS_CloseFile(&file);

			//Free memory.
			Heap::SetCurrent(Memory::rootHeapPtr);
			if (hInfo.numEvents)
				delete events;
			free(pStrmBuf);
		}
	}

	//Stops the music. (For external use ONLY)
	void stop(int frames)
	{
		stop(frames, false);
	}

	//Sets the event handler function.
	void setEventHandler(EventHandler func) { sInfo.eventHandler = func; }

	//Updates the events.
	void updateEvents(StreamInfo* sInfo)
	{
		if (!hInfo.numEvents || !sInfo->eventHandler)
			return;

		for (int i = 0; i < hInfo.numEvents; i++)
		{
			EventInfo info = events[i];
			if (info.sample > sInfo->musicCursor &&
				info.sample < sInfo->musicCursor + sInfo->samplesPerUpdate)
			{
				sInfo->eventHandler(info.eventID);
			}
		}
	}

	//Updates the music fading.
	bool updateFade()
	{
		if (sInfo.isPlaying && sInfo.fadeFrame)
		{
			int newVolume = sInfo.volume - sInfo.fadeDec;
			newVolume = MATH_CLAMP(newVolume, 0, 127);

			setVolume(newVolume, 0);
			sInfo.fadeFrame--;

			if (sInfo.fadeFrame == 0)
			{
				setVolume(sInfo.targetVolume, 0);
				if (sInfo.stopMode == 1)
				{
					stop(0, false);
					return false;
				}
			}
		}
		return true;
	}

	//Checks if the music has reached end or loop point and updates the music state accordingly.
	void updateCheckEnd(StreamInfo* sInfo, pStrmBufT pBuf, int len)
	{
		int leftOver = STRM_BUF_PAGESIZE - len;
		if (sInfo->loops)
		{
			if (sInfo->musicCursor > hInfo.loopEnd)
			{
				seek(hInfo.loopStart, true);
				if (leftOver > 0)
				{
					for (int i = 0; i < sInfo->chCount; i++)
						FS_ReadFile(&file, &(*pBuf)[i][len], leftOver);
				}
				seek(hInfo.loopStart + (leftOver / sInfo->bytesPerSample), true);
			}
		}
		else
		{
			if (sInfo->musicCursor > sInfo->musicEnd)
			{
				//Instead of instantly stopping the music, we must wait for the buffer end, otherwise the music will stop sooner.
				if (leftOver > 0)
				{
					for (int i = 0; i < sInfo->chCount; i++)
						MI_CpuClear8(&(*pBuf)[i][len], leftOver); //Clear the buffer end to avoid garble.
				}
				stop(0, true); //Stop the music and wait for the buffer end.
			}
		}
	}

	//Updates the music.
	void update(StreamInfo* sInfo)
	{
		//Check for delayed stop.
		if (sInfo->stopMode == 2)
		{
			stop(0, false);
			return;
		}

		//Get buffer page and swap.
		pStrmBufT pBuf = reinterpret_cast<pStrmBufT>(**pStrmBuf + (STRM_BUF_PAGESIZE * sInfo->bufPage));
		sInfo->bufPage = !sInfo->bufPage;

		//Get read length.
		int len = STRM_BUF_PAGESIZE;
		int limit = sInfo->loops ? hInfo.loopEnd : sInfo->musicEnd;
		int remain = (limit - sInfo->musicCursor) * sInfo->bytesPerSample;
		if (remain < len)
			len = remain;

		//Read the data to the buffer.
		for (int i = 0; i < sInfo->chCount; i++)
			FS_ReadFile(&file, (*pBuf)[i], len);

		//Increment the music cursor.
		sInfo->musicCursor += sInfo->samplesPerUpdate;

		//Update the events.
		updateCheckEnd(sInfo, pBuf, len);
		updateEvents(sInfo);
	}

	//The sound alarm function that unblocks the thread.
	static void SoundAlarmHandler(void* arg)
	{
		//Unblock updater thread.
		OS_SendMessage(&msgQ, (OSMessage)arg, OS_MESSAGE_NOBLOCK);
	}

	//Setups the music channels, timers and sound alarm.
	void setup()
	{
		//Calculate timer values.
		s32 timerValue = SND_TIMER_CLOCK / sInfo.playRate;
		u32 alarmPeriod = timerValue * (STRM_BUF_PAGESIZE / sInfo.bytesPerSample) / 32;

		//Setup channels.
		for (int i = 0; i < sInfo.chCount; i++)
		{
			bool left = i == 0;
			SND_SetupChannelPcm(
				left ? CHANNEL_L_NUM : CHANNEL_R_NUM,
				static_cast<SNDWaveFormat>(hInfo.format),
				left ? (*pStrmBuf)[0] : (*pStrmBuf)[1],
				SND_CHANNEL_LOOP_REPEAT,
				0,
				STRM_BUF_SIZE / sizeof(u32),
				sInfo.volume,
				SND_CHANNEL_DATASHIFT_NONE,
				timerValue,
				!hInfo.stereo ? 64 : (left ? 0 : 127)
			);
		}

		//Setup sound alarm for updater thread.
		SND_SetupAlarm(ALARM_NUM, alarmPeriod, alarmPeriod, SoundAlarmHandler, &sInfo);
	}

	//Gets the music speed.
	fx32 getSpeed() { return sInfo.speed; }

	//Sets the music speed.
	void setSpeed(fx32 speed)
	{
		//Set the music speed.
		fx32 sampleRate = hInfo.sampleRate << FX32_SHIFT;
		sInfo.playRate = FX_MulInline(sampleRate, speed) >> FX32_SHIFT;
		sInfo.speed = speed;

		//Update the music.
		bool notPaused = !sInfo.isPaused;
		if (notPaused)
			setPaused(true);
		setup();
		if (notPaused)
			setPaused(false);
	}

	//Loads the NWAV events that will be used to trigger the current callback function set.
	void loadEvents()
	{
		//Allocate events.
		events = new EventInfo[hInfo.numEvents];

		//Read event IDs.
		for (int i = 0; i < hInfo.numEvents; i++)
		{
			int val = 0;
			FS_ReadFile(&file, &val, 1);
			events[i].eventID = val;
		}

		//Jump to "samples for event IDs" block.
		seek(headerSize + sInfo.eventIDBlockSize, false);

		//Read samples for event IDs
		for (int i = 0; i < hInfo.numEvents; i++)
		{
			int val;
			FS_ReadFile(&file, &val, 4);
			events[i].sample = val;
		}
	}

	//Plays the music.
	void play(int fileID)
	{
		//If music is already playing, stop it.
		if (sInfo.isPlaying)
			stop(0, false);

		//Initialize file and try to open it, otherwise crash.
		FS_InitFile(&file);
		if (!FS_OpenFileFast(&file, (void*)0x02096114, fileID))
			OS_Panic();

		//Read the file header.
		FS_ReadFile(&file, &hInfo, headerSize);

		//Set the current memory loading heap.
		Heap::SetCurrent(Memory::rootHeapPtr);

		//Reset variables
		sInfo.loops = hInfo.loopEnd != 0;
		sInfo.fadeDec = 0;
		sInfo.fadeFrame = 0;
		sInfo.stopMode = 0;
		sInfo.volume = 127;
		sInfo.chCount = hInfo.stereo ? 2 : 1;
		sInfo.bytesPerSample = hInfo.format ? 2 : 1;
		sInfo.samplesPerUpdate = (STRM_BUF_PAGESIZE / sInfo.bytesPerSample);

		//Setup events.
		if (hInfo.numEvents)
		{
			int unalignedEvents = (hInfo.numEvents % 4);
			sInfo.eventIDBlockSize = hInfo.numEvents + (4 - unalignedEvents);
			sInfo.eventBlockSize = sInfo.eventIDBlockSize + (hInfo.numEvents * 4);
			loadEvents();
		}
		else
		{
			sInfo.eventIDBlockSize = 0;
			sInfo.eventBlockSize = 0;
		}

		//Calculate music size.
		sInfo.musicEnd = (((hInfo.fileSize - headerSize - sInfo.eventBlockSize) / sInfo.chCount) / sInfo.bytesPerSample);

		//Allocate stream buffer.
		pStrmBuf = reinterpret_cast<pStrmBufT>(malloc(STRM_BUF_SIZE * sInfo.chCount));

		//Start music read and updater.
		seek(0, true);
		setSpeed(sInfo.speed);

		sInfo.isPlaying = true;
		sInfo.isPaused = true;
		setPaused(false);
	}

	//The OS thread that runs the updater.
	static void StrmThread(void* arg)
	{
		OSMessage message;

		//Main thread loop
		while (true)
		{
			OS_ReceiveMessage(&msgQ, &message, OS_MESSAGE_BLOCK); //Block thread until message is received by the sound alarm.
			update(reinterpret_cast<StreamInfo*>(message));       //Update the music.
		}
	}

	//Initializes the NWAV player.
	void init()
	{
		//Lock the channels.
		SND_LockChannel(1 << CHANNEL_L_NUM | 1 << CHANNEL_R_NUM, 0);

		//Startup stream thread.
		OS_InitMessageQueue(&msgQ, &msgBuf, 1);
		OS_CreateThread(&strmThread,
			StrmThread,
			nullptr,
			&strmThreadStack[THREAD_STACK_SIZE],
			THREAD_STACK_SIZE,
			STREAM_THREAD_PRIO);
		OS_WakeupThreadDirect(&strmThread);
	}
}
