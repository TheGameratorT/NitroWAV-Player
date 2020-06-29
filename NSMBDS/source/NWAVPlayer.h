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
|  NWAV player core declarations.                             |
|  This is the header that allows the engine code to be       |
|  implemented into the game and be accessed externally.      |
\============================================================*/

#ifndef _NWAVPLAYER_H
#define _NWAVPLAYER_H

#define NWAV 0x5641574E

#include "nitro_if.h"

namespace NWAVPlayer
{
	//The function type of the function that will handle the events.
	typedef void(*EventHandler)(int);

	/// <summary>Initializes the player system. (Hook after SND_Init)</summary>
	/// <returns>True if the music wasn't stopped. False otherwise.</returns>
	void init();

	/// <summary>Updates the game fading. (Hook after SND_Main)</summary>
	/// <returns>True if the music wasn't stopped. False otherwise.</returns>
	bool updateFade();

	/// <summary>Plays a music.</summary>
	/// <param name="fileID">The file ID of the music file to play.</param>
	void play(int fileID);

	/// <summary>Stops the music playing.</summary>
	/// <param name="frames">Number of frames where the volume shift occurs.</param>
	void stop(int frames);

	/// <summary>Gets the music volume.</summary>
	/// <returns>The music volume.</returns>
	int getVolume();

	/// <summary>Sets the music volume.</summary>
	/// <param name="volume">The target volume. Value range = [0, 127]</param>
	/// <param name="frames">Number of frames where the volume shift occurs.</param>
	void setVolume(int volume, int frames);

	/// <summary>Gets the music speed.</summary>
	/// <returns>The current music speed.</returns>
	fx32 getSpeed();

	/// <summary>Sets the music speed.</summary>
	/// <param name="speed">The target speed for the music to be played at.</param>
	void setSpeed(fx32 speed);

	/// <summary>Gets if the music is paused.</summary>
	/// <returns>True if the music is paused. False otherwise.</returns>
	bool getPaused();

	/// <summary>Sets if the music is paused.</summary>
	/// <param name="paused">Sets the music as paused when true, unpauses when false.</param>
	void setPaused(bool paused);

	/// <summary>Sets the event handler function.</summary>
	/// <param name="func">The function pointer of the event handler.</param>
	void setEventHandler(EventHandler func);
}

#endif //!_NWAVPLAYER_H
