// SDK symbols the code reference does not name (or names wrongly), which the
// player needs from the game's own copy of the libraries.

// The decibel table SND_CalcDecibel reads.
asm(".globl SNDi_DecibelTable; SNDi_DecibelTable = 0x02084478");

// Named NNS_SndPlayerSetTrackPan_0 by the reference, but it is the track mute.
asm(".globl NWAVi_SndPlayerSetTrackMute; .type NWAVi_SndPlayerSetTrackMute, %function; NWAVi_SndPlayerSetTrackMute = 0x0205DC74");
