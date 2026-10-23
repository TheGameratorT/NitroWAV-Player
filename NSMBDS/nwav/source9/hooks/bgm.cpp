/*============================================================\
|  NWAV BGM replacement.                                      |
|  A sequence that has a stream in the BGM table never starts:|
|  the stream plays in its place. The game's controls for the |
|  music (volume and fades, pause, stop, tempo) are forwarded |
|  to the stream. With no sequence player, the game does not  |
|  copy the sequence variables either, so only the stream's   |
|  own "bahp" events reach the enemies.                       |
\============================================================*/

#include "nwav/hooks.hpp"

#include <nsmb/game/stage/player/common.hpp>
#include <nsmb/game/sound.hpp>

namespace NWAV::Hooks
{
	static Handle bgmStream = InvalidHandle;

#ifdef NWAV_BGM_BAHP
	// The game's "bahp" flag, which makes the enemies dance on the beat. A BGM
	// stream marks the beats with cue points named "bahp". The flag stays set for
	// one frame, as the sequence's own bahp did, and only in single player: there
	// the enemies that read it (see CoopFixes::Blockhopper) use their own timing.
	static u8& bahpFlag = *rcast<u8*>(0x02088B9C);
	static bool bahpRaised;
#endif

	[[gnu::weak]] void onBGMEvent(Handle, u32)
	{
	}

	static void dispatchBGMEvent(Handle handle, u32 eventID, void*)
	{
#ifdef NWAV_BGM_BAHP
		if (eventID == EventID("bahp") && Game::getPlayerCount() == 1)
		{
			bahpFlag = 1;
			bahpRaised = true;
		}
#endif
		onBGMEvent(handle, eventID);
	}

	[[gnu::weak]] const BGMEntry* getBGMTable(u32& count)
	{
		count = 0;
		return nullptr;
	}

	static const BGMEntry* findEntry(u32 seqID)
	{
		u32 count;
		const BGMEntry* table = getBGMTable(count);
		for (u32 i = 0; i < count; i++)
		{
			if (table[i].seqID == seqID)
				return &table[i];
		}
		return nullptr;
	}

	Handle getBGMStream()
	{
		return bgmStream;
	}

	void updateBGM()
	{
#ifdef NWAV_BGM_BAHP
		// Called right before NWAV::update dispatches this frame's events.
		if (bahpRaised)
		{
			bahpFlag = 0;
			bahpRaised = false;
		}
#endif

		if (!NWAV::isPlaying(bgmStream))
			bgmStream = InvalidHandle;
	}

	// The call to SND::Internal::startSeq(id) in Sound::startSeq, its only caller.
	// Replaced sequences start their stream instead of the sequence.
	ncp_call(0x02011ECC)
	BOOL NWAV_startSeq(int seqID)
	{
		const BGMEntry* entry = findEntry(seqID);
		NWAV_LOG("NWAV startSeq: seqID=" << seqID << " entry=" << entry << "\n");

		// Whatever played before is replaced, a sequence or a stream.
		NWAV::stop(bgmStream);
		bgmStream = InvalidHandle;

		if (entry)
		{
			SND::Internal::stopSeq(0);
			bgmStream = playExtFile(entry->fileID, { .priority = BGMPriority, .onEvent = dispatchBGMEvent });
			if (bgmStream != InvalidHandle)
			{
				NWAV::setDecibelOffset(bgmStream, entry->decibels);
				return TRUE;
			}
			NWAV_LOG("NWAV startSeq: stream failed, the sequence plays instead\n");
		}

		return SND::Internal::startSeq(seqID);
	}

	// The game's music controls start with "STMFD SP!, {R4,LR}". Each one is replaced
	// by a jump to a function below, which runs the original through a copy of that
	// instruction and a branch past it, then does the same to the stream.
	ncp_asmfunc static void fadeSeqVolumeOriginal(int volume, int frames) { asm("STMFD SP!, {R4,LR}\nB 0x0204DCB8"); }
	ncp_asmfunc static void stopSeqOriginal(int frames) { asm("STMFD SP!, {R4,LR}\nB 0x0204E424"); }
	ncp_asmfunc static void pauseSeqOriginal(BOOL paused) { asm("STMFD SP!, {R4,LR}\nB 0x0204E480"); }
	ncp_asmfunc static BOOL setTempoRatioOriginal(int ratio) { asm("STMFD SP!, {R4,LR}\nB 0x0204E530"); }

	// SND::Internal::fadeSeqVolume(volume, frames)
	ncp_jump(0x0204DCB4)
	void NWAV_fadeSeqVolume(int volume, int frames)
	{
		fadeSeqVolumeOriginal(volume, frames);
		NWAV::setVolume(bgmStream, volume, frames);
	}

	// SND::Internal::stopSeq(frames)
	ncp_jump(0x0204E420)
	void NWAV_stopSeq(int frames)
	{
		stopSeqOriginal(frames);
		NWAV::stop(bgmStream, frames);
	}

	// SND::Internal::pauseSeq(paused)
	ncp_jump(0x0204E47C)
	void NWAV_pauseSeq(BOOL paused)
	{
		pauseSeqOriginal(paused);
		NWAV::setPaused(bgmStream, paused);
	}

#ifdef NWAV_BGM_FOLLOW_TEMPO
	// SND::Internal::setTempoRatio(ratio), where 256 is the sequence's own tempo.
	ncp_jump(0x0204E52C)
	BOOL NWAV_setTempoRatio(int ratio)
	{
		BOOL result = setTempoRatioOriginal(ratio);
		NWAV::setSpeed(bgmStream, ratio << (FX32_SHIFT - 8));
		return result;
	}
#endif
}
