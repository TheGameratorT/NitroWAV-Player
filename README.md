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
 - Events come from `cue` points, and the cue ID is the event ID.
 - Rates up to 32728 Hz, which is the rate the DS mixes at.

Use [nwavtool](https://github.com/TheGameratorT/NitroWAV-Tool) to convert audio
into these formats, to check a file, or to migrate old `.nwav` files.

## Playing a file

```cpp
#include "nwav/nwav.hpp"

NWAV::Handle h = NWAV::play("music/boss.wav"fid);
NWAV::setVolume(h, 64, 30); // Fade to half volume over 30 frames.
NWAV::stop(h, 60);          // Fade out over 60 frames, then stop.
```

`PlayParams` sets the volume, pan, priority, loop mode, speed, start frame,
fade-in and event handler. See `NSMBDS/nwav/include/nwav/nwav.hpp`.

## Components

The core in `source9/core` only depends on the SDK. Everything specific to the
game is a component (see `NSMBDS/nwav/module.yaml`), which a project can switch off to hook things its own way.

| Component          | What it does |
|--------------------|--------------|
| `Config`           | Stream count, buffer sizes and thread priority (see `NSMBDS/nwav/include/nwav/config.hpp`). |
| `Hooks`            | Starts the player with the sound system and updates it once per frame. |
| `BGMReplace`       | Plays a WAV in place of the game's BGM sequences, following their volume, fades, pauses, crossfades and stops. |
| `BGMFollowTempo`   | Speeds the stream up along with the hurry-up tempo. |
| `SceneIntegration` | Pauses streams when the game pauses its sound, and stops non-BGM streams on a scene switch. |

To replace BGM, define the table anywhere in your project:

```cpp
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
