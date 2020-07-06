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
|  NWAV player core declarations.                             |
|  This is the header that allows the engine code to be       |
|  implemented into the game and be accessed externally.      |
\============================================================*/

#ifndef _NWAVPLAYER_H
#define _NWAVPLAYER_H

#define NWAV 0x5641574E

#include "mkds.h"

#ifdef __cplusplus
extern "C" {
#endif

	//The function type of the function that will handle the events.
	typedef void(*NWAVEventHandler)(int);

	/// <summary>Plays a music.</summary>
	/// <param name="fileID">The file ID of the music file to play.</param>
	/// <param name="speed">Sets the speed to be played. (0 = do not change)</param>
	/// <param name="volume">Sets the volume to be played. (0 = do not change)</param>
	/// <param name="resume">The speed scale related to the music playing. (0 = do not resume)</param>
	void NWAV_Play(int fileID, fx32 speed, int volume, fx32 resume);

	/// <summary>Stops the music playing.</summary>
	/// <param name="frames">Number of frames where the volume shift occurs.</param>
	void NWAV_Stop(int frames);

	/// <summary>Gets the music volume.</summary>
	/// <returns>The music volume.</returns>
	int NWAV_GetVolume();

	/// <summary>Sets the music volume.</summary>
	/// <param name="volume">The target volume. Value range = [0, 127]</param>
	/// <param name="frames">Number of frames where the volume shift occurs. (0 = instant change)</param>
	void NWAV_SetVolume(int volume, int frames);

	/// <summary>Gets the music speed.</summary>
	/// <returns>The current music speed.</returns>
	fx32 NWAV_GetSpeed();

	/// <summary>Sets the music speed.</summary>
	/// <param name="speed">The target speed for the music to be played at.</param>
	void NWAV_SetSpeed(fx32 tempo);

	/// <summary>Sets either if the music pitch waves up and down.</summary>
	/// <param name="waving">True if should wave. False otherwise.</param>
	/// <param name="returnSpeed">The destination speed when the waving is stopped.</param>
	void NWAV_SetPitchWaving(bool waving, fx32 returnSpeed);

	/// <summary>Gets if the music is paused.</summary>
	/// <returns>True if the music is paused. False otherwise.</returns>
	bool NWAV_GetPaused();

	/// <summary>Sets if the music is paused.</summary>
	/// <param name="paused">Sets the music as paused when true, unpauses when false.</param>
	void NWAV_SetPaused(bool paused);

	/// <summary>Sets the event handler function.</summary>
	/// <param name="func">The function pointer of the event handler.</param>
	void NWAV_SetEventHandler(NWAVEventHandler func);

#ifdef __cplusplus
}
#endif

#endif //!_NWAVPLAYER_H
