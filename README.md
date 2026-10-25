# NitroWAV Player
Streams music and sound from the ROM on a DS.

# Supported Games

| Game                     | State             | Format | Patch Requirements |
| -------------------------|-------------------|--------|--------------------|
| New Super Mario Bros. DS | Receiving updates | WAV    | [NCPatcher](https://github.com/TheGameratorT/NCPatcher) 2.x module, [NSMB Code Reference](https://github.com/MammaMiaTeam/NSMB-Code-Reference) |
| Mario Kart DS            | Legacy            | NWAV   |                    |
| Super Mario 64 DS        | Not started       |        |                    |

# New Super Mario Bros. DS

`NSMBDS/nwav` is a module for [NCPatcher](https://github.com/TheGameratorT/NCPatcher)
v2. It relies on NCPatcher v2's module system (`module.yaml`, components and
`nitrofs`), so NCPatcher 1.x cannot build it. Your project also needs the
[NSMB Code Reference](https://github.com/MammaMiaTeam/NSMB-Code-Reference) and
the converted Nitro SDK headers.

To install it, copy the folder into your project's modules directory as
`modules/nwav`, then add `nwav` to `modules.enabled` in `ncpatcher.yaml`:

```yaml
modules:
  dir: modules
  enabled:
    - nwav
    # Components can be switched off or configured per project:
    # - nwav:
    #     components:
    #       BGMFollowTempo: { enabled: false }
```

It streams plain RIFF WAV files from NitroFS:
 - PCM8, PCM16 or IMA-ADPCM, mono or stereo.
 - The loop comes from the `smpl` chunk and ends on the exact frame you set,
   so loops are seamless.
 - Events come from `cue` points. An event has a name such as `bahp`, and its cue
   ID is the FNV-1a hash of the name. The handler gets that ID, and
   `NWAV::EventID("bahp")` computes it at compile time to compare with:
   `if (eventID == NWAV::EventID("bahp"))`.
 - Rates up to 32728 Hz, which is the rate the DS mixes at.

Use [nwavtool](https://github.com/TheGameratorT/NitroWAV-Tool) to convert audio
into these formats, to check a file, or to migrate old `.nwav` files.

## Playing a file

```cpp
#include "fid.hpp"
#include "nwav/hooks.hpp"

NWAV::Handle h = NWAV::Hooks::playExtFile("music/boss.wav"fid);
NWAV::setVolume(h, 64, 30); // Fade to half volume over 30 frames.
NWAV::stop(h, 60);          // Fade out over 60 frames, then stop.

// A sound effect, which stops when the game stops its own (SoundIntegration).
NWAV::Hooks::playExtFile("voice/yahoo.wav"fid, { .tags = NWAV::Hooks::SFXTag });
```

`NWAV::play` takes a NitroFS file ID, which counts the game's 131 overlays, while
the `fid` literal gives the game's extended ID. `playExtFile` adds the offset.

`PlayParams` sets the volume, pan, priority, tags, loop
mode, speed, start frame, fade-in, event handler and a user data pointer, which `NWAV::getUserData` reads back from the handle. See `NSMBDS/nwav/include/nwav/nwav.hpp`.

## Components

The core in `source9/core` only depends on the SDK. Everything specific to the
game is a component (see `NSMBDS/nwav/module.yaml`), which a project can switch off to hook things its own way.

| Component          | What it does |
|--------------------|--------------|
| `Config`           | Stream count, buffer sizes and thread priority (see `NSMBDS/nwav/include/nwav/config.hpp`). |
| `Hooks`            | Starts the player with the sound system and updates it once per frame. |
| `FSInit`           | Makes the game read the card with the CPU instead of DMA channel 2, which the background HDMA interferes with while streaming. |
| `BGMReplace`       | Plays a WAV in place of the game's BGM sequences, following their volume, fades, pauses, crossfades and stops. |
| `BGMFollowTempo`   | Speeds the stream up along with the hurry-up tempo. |
| `BGMBahp`          | Raises the game's "bahp" flag (the enemies' dance beat) for a frame on each `bahp` event of a BGM stream, in single player. |
| `SoundIntegration` | Pauses streams when the game pauses its sound, and stops the streams played as sound effects (tagged `NWAV::Hooks::SFXTag`) whenever the game stops its own, as scenes do when they end. |

The game's music functions are hooked at their entry, not at their calls, so code
that calls them directly (other hacks included) controls the BGM stream too.

To start the player from your own hooks, switch `Hooks` off, call `NWAV::init`
once the game's sound system is up, and call `NWAV::Hooks::update()` once per
frame before `NNS_SndMain`. Call it in place of `NWAV::update`, because it also
steps the BGM components. `playExtFile` and the other components keep working.

The module needs `-fno-short-enums` in your common compiler flags, because the
game's structs were built with 4 byte enums. Define `NWAV_DEBUG` to log what the
player does through `Log`, which prints in `NTR_DEBUG` builds.

To replace BGM, define the table anywhere in your project:

```cpp
#include "fid.hpp"
#include "nwav/hooks.hpp"
#include <nsmb/game/sound.hpp>

static const NWAV::Hooks::BGMEntry bgmTable[] = {
	// Sequence,         WAV file,                 attenuation (tenths of a dB)
	{ SeqID::ATHLETIC,   "music/athletic.wav"fid,  0 },
};

const NWAV::Hooks::BGMEntry* NWAV::Hooks::getBGMTable(u32& count)
{
	count = sizeof(bgmTable) / sizeof(bgmTable[0]);
	return bgmTable;
}
```

To react to the events of the BGM streams, define `onBGMEvent` anywhere in your
project the same way. It is called on the main thread for every event, after the
module's own handling of it (`BGMBahp`), so the module never needs editing. The
`eventID` is the hash of the name given in nwavtool, which `NWAV::EventID`
computes at compile time:

```cpp
#include "nwav/hooks.hpp"

void NWAV::Hooks::onBGMEvent(NWAV::Handle handle, u32 eventID)
{
	if (eventID == NWAV::EventID("flash"))
		flashTheScreen();
	else if (eventID == NWAV::EventID("boom"))
		shakeTheCamera();
}
```
