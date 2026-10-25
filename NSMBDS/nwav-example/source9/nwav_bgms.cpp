#include "nwav/hooks.hpp"
#include <nsmb/game/sound.hpp>
#include <fid.hpp>

static const NWAV::Hooks::BGMEntry bgmTable[] = {
	// Sequence, WAV file,                             Hurry WAV file,                            Attenuation (tenths of a dB)
	//{ 1,       "nwav-example/tower.wav"fid,          NO_HURRY_TRACK,                            0 },
	{ 26,        "nwav-example/ground_theme.wav"fid,   "nwav-example/ground_theme_hurry.wav"fid,  0 },
};

const NWAV::Hooks::BGMEntry* NWAV::Hooks::getBGMTable(u32& count)
{
	count = sizeof(bgmTable) / sizeof(bgmTable[0]);
	return bgmTable;
}
