/*============================================================\
|  NWAV pause and scene hooks.                                |
|  Streams pause with the game's sound and the ones below the |
|  BGM priority stop when the scene changes.                  |
\============================================================*/

#include "nwav/hooks.hpp"

#ifndef NWAV_SCENE_FADE_FRAMES
#define NWAV_SCENE_FADE_FRAMES 0
#endif

namespace NWAV::Hooks
{
	// SND::Internal::pauseSeq, which pauses every sound player.
	ncp_call(0x0204E4BC)
	void NWAV_pauseAll(BOOL paused)
	{
		NNS_SndPlayerPauseAll(paused); // Keep replaced instruction
		NWAV::setPausedAll(paused);
	}

	// Scene::switchScene, once the switch is accepted.
	ncp_hook(0x02013234)
	void NWAV_switchScene()
	{
		NWAV::stopAll(NWAV_SCENE_FADE_FRAMES, BGMPriority);
	}
}
