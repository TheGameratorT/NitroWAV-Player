/*============================================================\
|  NWAV game integration for New Super Mario Bros.            |
|  Declarations shared by the module's hook components.       |
\============================================================*/

#pragma once

#include "nwav/nwav.hpp"

namespace NWAV::Hooks
{
	// Priority of streams that replace the game's BGM. Streams below it are
	// stopped on scene switches and can never take a BGM stream's slot.
	constexpr u8 BGMPriority = 128;

	struct BGMEntry
	{
		u32 seqID;		// The sequence to replace.
		u32 fileID;		// The WAV file that plays instead.
		s32 decibels;	// Attenuation in tenths of a decibel. Value range = [-723, 0]
	};

	/// <summary>
	/// Gets the BGM replacement table. The module defines it weakly as empty;
	/// a project replaces it by defining this function.
	/// </summary>
	const BGMEntry* getBGMTable(u32& count);

	/// <summary>Mirrors the game's BGM sequence players onto streams. (Called by the frame hook)</summary>
	void updateBGM();

	/// <summary>Gets the stream replacing a BGM sequence player, if any.</summary>
	/// <param name="playerIndex">The index into SND::Internal::seqPlayerPool.</param>
	Handle getBGMStream(u32 playerIndex);
}
