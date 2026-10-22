/*============================================================\
|  RIFF WAV header parser.                                    |
|  Walks the chunks once when a stream opens and keeps what   |
|  the player needs: fmt, fact, data, smpl and cue.           |
\============================================================*/

#include "stream.hpp"


namespace NWAV::Core
{
	constexpr u32 fourCC(const char (&s)[5])
	{
		return u32(s[0]) | (u32(s[1]) << 8) | (u32(s[2]) << 16) | (u32(s[3]) << 24);
	}

	constexpr u16 WaveFormatPcm = 0x0001;
	constexpr u16 WaveFormatIma = 0x0011;

	static u32 readU32(const u8* p)
	{
		return u32(p[0]) | (u32(p[1]) << 8) | (u32(p[2]) << 16) | (u32(p[3]) << 24);
	}

	static u16 readU16(const u8* p)
	{
		return u16(p[0] | (p[1] << 8));
	}

	static bool readAt(FSFile* file, u32 offset, void* dst, s32 len)
	{
		if (!FS_SeekFile(file, s32(offset), FS_SEEK_SET))
			return false;
		return FS_ReadFile(file, dst, len) == len;
	}

	static bool parseFmt(FSFile* file, u32 offset, u32 size, Info& info)
	{
		u8 fmt[20];
		if (size < 16 || !readAt(file, offset, fmt, size < 20 ? 16 : 20))
			return false;

		u16 tag = readU16(&fmt[0]);
		u16 channels = readU16(&fmt[2]);
		u16 bits = readU16(&fmt[14]);

		info.channels = u8(channels);
		info.sampleRate = readU32(&fmt[4]);
		info.blockAlign = readU16(&fmt[12]);

		if (channels < 1 || channels > 2 || info.sampleRate == 0)
			return false;

		if (tag == WaveFormatPcm && (bits == 8 || bits == 16))
		{
			info.format = bits == 8 ? Format::Pcm8 : Format::Pcm16;
			return info.blockAlign == channels * (bits / 8);
		}

		if (tag == WaveFormatIma && bits == 4 && size >= 20)
		{
			// Each block holds a 4 byte header per channel, then 4 byte words
			// of 8 samples alternating between the channels.
			u32 unit = 4 * channels;
			if (info.blockAlign <= unit || info.blockAlign % unit != 0)
				return false;

			info.format = Format::Ima;
			info.framesPerBlock = u16(1 + (info.blockAlign - unit) / unit * 8);
			return readU16(&fmt[18]) == info.framesPerBlock;
		}

		return false;
	}

	static void parseSmpl(FSFile* file, u32 offset, u32 size, Info& info)
	{
		// The first loop follows a 36 byte header: id, type, start, end (inclusive).
		u8 smpl[52];
		if (size < sizeof(smpl) || !readAt(file, offset, smpl, sizeof(smpl)))
			return;
		if (readU32(&smpl[28]) == 0)
			return;

		info.loopStart = readU32(&smpl[44]);
		info.loopEnd = readU32(&smpl[48]) + 1;
		info.hasLoop = true;
	}

	static void parseCue(FSFile* file, u32 offset, u32 size, Info& info)
	{
		// Each cue point: name (used as the event ID), position, chunk,
		// chunk start, block start and sample offset.
		u8 entry[24];
		if (size < 4 || !readAt(file, offset, entry, 4))
			return;

		u32 count = readU32(entry);
		if (count > (size - 4) / 24)
			count = (size - 4) / 24;
		if (count > NWAV_MAX_EVENTS)
			count = NWAV_MAX_EVENTS;

		for (u32 i = 0; i < count; i++)
		{
			if (FS_ReadFile(file, entry, 24) != 24)
				break;

			Event& event = info.events[info.numEvents++];
			event.id = readU32(&entry[0]);
			event.frame = readU32(&entry[20]);
		}
	}

	bool parseWav(FSFile* file, Info& info)
	{
		MI_CpuFill8(&info, 0, sizeof(Info));

		u32 fileSize = FS_GetLength(file);
		u8 header[12];
		if (!readAt(file, 0, header, 12) ||
			readU32(&header[0]) != fourCC("RIFF") ||
			readU32(&header[8]) != fourCC("WAVE"))
			return false;

		bool hasFmt = false;
		bool hasData = false;
		u32 factFrames = 0;

		u32 offset = 12;
		while (offset + 8 <= fileSize)
		{
			if (!readAt(file, offset, header, 8))
				return false;

			u32 id = readU32(&header[0]);
			u32 size = readU32(&header[4]);
			u32 body = offset + 8;
			if (size > fileSize - body)
				size = fileSize - body;

			if (id == fourCC("fmt "))
			{
				if (!parseFmt(file, body, size, info))
				{
					NWAV_LOG("NWAV wav: unsupported fmt chunk\n");
					return false;
				}
				hasFmt = true;
			}
			else if (id == fourCC("data"))
			{
				info.dataOffset = body;
				info.dataSize = size;
				hasData = true;
			}
			else if (id == fourCC("fact") && size >= 4)
			{
				u8 fact[4];
				if (readAt(file, body, fact, 4))
					factFrames = readU32(fact);
			}
			else if (id == fourCC("smpl"))
			{
				parseSmpl(file, body, size, info);
			}
			else if (id == fourCC("cue "))
			{
				parseCue(file, body, size, info);
			}

			offset = body + size + (size & 1);
		}

		if (!hasFmt || !hasData)
		{
			NWAV_LOG("NWAV wav: missing chunk, fmt=" << u32(hasFmt) << " data=" << u32(hasData) << "\n");
			return false;
		}

		if (info.format == Format::Ima)
		{
			// Keep whole words so reads never split one.
			u32 unit = 4 * info.channels;
			info.dataSize -= info.dataSize % unit;

			u32 blocks = info.dataSize / info.blockAlign;
			u32 rest = info.dataSize % info.blockAlign;
			info.numFrames = blocks * info.framesPerBlock;
			if (rest >= unit)
				info.numFrames += 1 + (rest - unit) / unit * 8;

			if (factFrames && factFrames < info.numFrames)
				info.numFrames = factFrames;
		}
		else
		{
			info.dataSize -= info.dataSize % info.blockAlign;
			info.numFrames = info.dataSize / info.blockAlign;
		}

		if (info.numFrames == 0)
			return false;

		if (info.hasLoop)
		{
			if (info.loopEnd > info.numFrames)
				info.loopEnd = info.numFrames;
			if (info.loopStart >= info.loopEnd)
				info.hasLoop = false;
		}

		return true;
	}
}
