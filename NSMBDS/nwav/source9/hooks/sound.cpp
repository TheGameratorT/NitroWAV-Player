/*============================================================\
|  NWAV sound system hooks.                                   |
|  Starts the player with the game's sound system and steps   |
|  it once per frame, right before NNS_SndMain flushes the    |
|  frame's sound commands to the ARM7.                        |
\============================================================*/

#include "nwav/hooks.hpp"

#include <nsmb/core/system/memory.hpp>

static void* NWAV_allocate(u32 size)
{
	return Memory::rootHeapPtr->allocate(size, 32);
}

static void NWAV_deallocate(void* ptr)
{
	Memory::rootHeapPtr->deallocate(ptr);
}

// The core takes NitroFS file IDs, which come after the game's 131 overlays.
constexpr u32 FileIDOffset = 131;

NWAV::Handle NWAV::Hooks::playExtFile(u32 extFileID, const PlayParams& params)
{
	NWAV_LOG("NWAV playExtFile: extFileID=" << extFileID << " fileID=" << (extFileID + FileIDOffset) << "\n");
	NWAV::Handle handle = NWAV::play(extFileID + FileIDOffset, params);
	NWAV_LOG("NWAV playExtFile: handle=" << handle << "\n");
	return handle;
}

#ifdef NWAV_HOOK_FS_INIT
// InitGame's FS_Init setup chooses DMA channel 2 for card reads, unless this is a
// multiboot child, where it passes -1 and the CPU copies instead. Streaming reads
// the card all the time, and a DMA read gets interfered with by the background
// HDMA, so the branch to the DMA choice is never taken.
ncp_repl(0x02009434, "NOP")
#endif

// SND::Internal::createHeap
ncp_call(0x0204F2F0)
void NWAV_hookSndInit()
{
	NNS_SndInit(); // Keep replaced instruction
	NWAV_LOG("NWAV hookSndInit\n");
	NWAV::init({ NWAV_allocate, NWAV_deallocate });
}

// SND::Internal::updateSound
ncp_call(0x0204EC74)
void NWAV_hookSndMain()
{
#ifdef NWAV_HOOK_BGM
	NWAV::Hooks::updateBGM();
#endif
	NWAV::update();
	NNS_SndMain(); // Keep replaced instruction
}
