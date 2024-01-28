# NitroWAV Player
Allows streamed music to be played on a DS.

This player which relies on a custom WAV format which I named NWAV, allows streamed music to play on a DS game without stressing the CPU too much.

# Supported Games

| Game                     | State              | Patch Requirements |
| -------------------------|--------------------|--------------------|
| New Super Mario Bros. DS | No Longer Updating | [NSMB-ASMReference](https://github.com/Overblade/NSMB-ASMReference) |
| Mario Kart DS            | No Longer Updating |                    |
| Super Mario 64 DS        | Not started        |                    |

# Supported WAV formats
The source WAV file that is fed to the converter must follow these specifications:
 - PCM-8 or PCM-16 format
 - Max sample rate of 32728 Hz
 - Have a max of 2 channels (Mono or Stereo)
 - (Optional) Can include "loopStart" and "loopEnd" markers corresponding to the samples, or WAV loop points.
 - (Optional) Can include event markers.

Note: All of the optional settings can be set in the converter.
