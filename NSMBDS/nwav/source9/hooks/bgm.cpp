/*============================================================\
|  NWAV BGM replacement.                                      |
|  The game keeps playing its BGM sequences, muted, so all of |
|  its music logic (crossfades, fades, pauses, hurry up) runs |
|  untouched. Each frame the sequence players are mirrored    |
|  onto streams: volume, pause, tempo and stop.               |
\============================================================*/

#include "nwav/hooks.hpp"

#include <nsmb/game/sound.hpp>

extern "C" void NWAVi_SndPlayerSetTrackMute(NNSSndHandle* handle, u16 trackBitMask, BOOL flag);

namespace NWAV::Hooks
{
	constexpr u32 PlayerCount = 2;
	constexpr s32 TempoRatioOne = 256;

	struct Binding
	{
		NNSSndSeqPlayer* seqPlayer;
		const BGMEntry* entry;
		Handle stream;
		s32 tempoRatio;
		bool started;
	};

	static Binding bindings[PlayerCount];

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

	static s32 findPlayer(const NNSSndHandle* handle)
	{
		for (u32 i = 0; i < PlayerCount; i++)
		{
			if (handle == &SND::Internal::seqPlayerPool[i].handle)
				return s32(i);
		}
		return -1;
	}

	static void unbind(Binding& b)
	{
		NWAV::stop(b.stream);
		b.seqPlayer = nullptr;
		b.entry = nullptr;
		b.stream = InvalidHandle;
	}

	Handle getBGMStream(u32 playerIndex)
	{
		return playerIndex < PlayerCount ? bindings[playerIndex].stream : InvalidHandle;
	}

	void updateBGM()
	{
		for (u32 i = 0; i < PlayerCount; i++)
		{
			Binding& b = bindings[i];
			NNSSndHandle* handle = &SND::Internal::seqPlayerPool[i].handle;
			NNSSndSeqPlayer* seqPlayer = handle->player;

			if (b.started)
			{
				// A new sequence started on this player, even if it is the
				// same one again: restart the stream from the beginning.
				b.started = false;
				unbind(b);

				NWAV_LOG("NWAV BGM: player " << i << " started, seqPlayer=" << seqPlayer
					<< " seqType=" << u32(seqPlayer ? seqPlayer->seqType : 0) << " seqNo=" << u32(seqPlayer ? seqPlayer->seqNo : 0) << "\n");

				if (seqPlayer && seqPlayer->seqType == NNS_SND_PLAYER_SEQ_TYPE_SEQ)
					b.entry = findEntry(seqPlayer->seqNo);
				NWAV_LOG("NWAV BGM: entry=" << b.entry << "\n");
				if (b.entry)
				{
					b.seqPlayer = seqPlayer;
					b.stream = playExtFile(b.entry->fileID, { .priority = BGMPriority });
					if (b.stream == InvalidHandle)
					{
						NWAV_LOG("NWAV BGM: stream failed, the sequence plays instead\n");
						b.entry = nullptr; // Let the sequence play rather than go silent.
					}
				}
			}

			if (!b.entry)
				continue;

			// The sequence stopped, or finished fading out.
			if (seqPlayer != b.seqPlayer)
			{
				NWAV_LOG("NWAV BGM: player " << i << " sequence stopped, unbinding\n");
				unbind(b);
				continue;
			}

			// Mute every frame, since the game unmutes tracks on its own (World 8).
			NWAVi_SndPlayerSetTrackMute(handle, 0xFFFF, TRUE);

			if (!NWAV::isPlaying(b.stream))
				continue;

			// The summed sequence volume of the previous frame: initial, external,
			// player and fader, in tenths of a decibel.
			NWAV::setDecibelOffset(b.stream, seqPlayer->volume + b.entry->decibels);
			NWAV::setPaused(b.stream, seqPlayer->pauseFlag);
#ifdef NWAV_BGM_FOLLOW_TEMPO
			NWAV::setSpeed(b.stream, b.tempoRatio << (FX32_SHIFT - 8));
#endif
		}
	}

	static void onSeqStarted(NNSSndHandle* handle)
	{
		s32 i = findPlayer(handle);
		if (i < 0)
			return;
		bindings[i].started = true;
		bindings[i].tempoRatio = TempoRatioOne;
	}

	// The three places the game starts a BGM sequence.
	BOOL NWAV_startSeq(NNSSndHandle* handle, int seqNo)
	{
		BOOL started = NNS_SndArcPlayerStartSeq(handle, seqNo);
		NWAV_LOG("NWAV startSeq: seqNo=" << seqNo << " started=" << started << " player=" << findPlayer(handle) << "\n");
		if (started)
			onSeqStarted(handle);
		return started;
	}

	// SND::Internal::setTempoRatio(NNSSndHandle*, s32), the only caller.
	ncp_call(0x0204E4F4)
	void NWAV_setTempoRatio(NNSSndHandle* handle, int ratio)
	{
		NNS_SndPlayerSetTempoRatio(handle, ratio); // Keep replaced instruction
		s32 i = findPlayer(handle);
		if (i >= 0)
			bindings[i].tempoRatio = ratio;
	}
}

ncp_set_call(0x0204E6BC, NWAV::Hooks::NWAV_startSeq) // SND::Internal::startSeq, fanfares
ncp_set_call(0x0204E748, NWAV::Hooks::NWAV_startSeq) // SND::Internal::startSeq, star/switch/mega
ncp_set_call(0x0204E970, NWAV::Hooks::NWAV_startSeq) // SND::Internal::updateSound, after a crossfade
