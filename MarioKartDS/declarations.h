/*===============================================================\
|  Simplified definitions of which declarations are required.    |
|  This is meant for the code to be compilable without the SDK.  |
\===============================================================*/

#define FX_Mul(v1, v2) FX_MulFunc(v1, v2)
#define FX32_SHIFT 12
#define MATH_CLAMP(x, low, high) ( ( (x) > (high) ) ? (high) : ( ( (x) < (low) ) ? (low) : (x) ) )

typedef u8 FSFile[72];
typedef u8 OSThread[200];
typedef u8 OSMessageQueue[32];

typedef void *OSMessage;
typedef void (*SNDAlarmHandler)(void*);

typedef enum
{
	SND_CHANNEL_DATASHIFT_NONE,
	SND_CHANNEL_DATASHIFT_1BIT,
	SND_CHANNEL_DATASHIFT_2BIT,
	SND_CHANNEL_DATASHIFT_4BIT
} SNDChannelDataShift;

typedef enum
{
	SND_WAVE_FORMAT_PCM8,
	SND_WAVE_FORMAT_PCM16,
	SND_WAVE_FORMAT_ADPCM,
	SND_WAVE_FORMAT_PSG,
	SND_WAVE_FORMAT_NOISE = SND_WAVE_FORMAT_PSG
} SNDWaveFormat;

typedef enum
{
	SND_CHANNEL_LOOP_MANUAL,
	SND_CHANNEL_LOOP_REPEAT,
	SND_CHANNEL_LOOP_1SHOT
} SNDChannelLoop;

typedef enum
{
	FS_SEEK_SET,
	FS_SEEK_CUR,
	FS_SEEK_END
} FSSeekFileMode;

extern "C"
{
	void OS_Terminate();
	void OS_WakeupThreadDirect(OSThread *thread);
	void OS_CreateThread(OSThread *thread, void (*func)(void *), void *arg, void *stack, u32 stackSize, u32 prio);
	BOOL OS_ReceiveMessage(OSMessageQueue *mq, OSMessage *msg, s32 flags);
	BOOL OS_SendMessage(OSMessageQueue *mq, OSMessage msg, s32 flags);
	void OS_InitMessageQueue(OSMessageQueue *mq, OSMessage *msgArray, s32 msgCount);
	
	void MI_CpuFill8(void *dest, u8 data, u32 size);
	static inline void MI_CpuClear8(void *dest, u32 size) {
		MI_CpuFill8(dest, 0, size);
	}
	
	void SND_SetupChannelPcm(int chNo, SNDWaveFormat format, const void *dataAddr, SNDChannelLoop loop, int loopStart, int dataLen, int volume, SNDChannelDataShift shift, int timer, int pan);
	void SND_SetChannelVolume(u32 chBitMask, int volume, SNDChannelDataShift shift);
	void SND_LockChannel(u32 chBitMask, u32 flags);
	void SND_SetupAlarm(int alarmNo, u32 tick, u32 period, SNDAlarmHandler handler, void *arg);
	void SND_StopTimer(u32 chBitMask, u32 capBitMask, u32 alarmBitMask, u32 flags);
	void SND_StartTimer(u32 chBitMask, u32 capBitMask, u32 alarmBitMask, u32 flags);
	
	BOOL FS_SeekFile(FSFile *p_file, s32 offset, FSSeekFileMode origin);
	s32  FS_ReadFile(FSFile *p_file, void *dst, s32 len);
	BOOL FS_CloseFile(FSFile *p_file);
	BOOL FS_OpenFileFast(FSFile* p_file, void* archivePtr, int file_id);
	void FS_InitFile(FSFile *p_file);
	
	fx32 FX_MulFunc(fx32 v1, fx32 v2);
}

static inline s64 FX_Mul64(s64 v1, s64 v2) {
	return ((s64)v1 * (s64)v2 + 0x800LL) >> FX32_SHIFT;
}

static inline void* malloc(size_t size) {
	void* process = Process_getCurrent();
	void* handle = Process_getHeapHandle(process);
	return MKDS_Alloc(handle, size);
}

static inline void* calloc(size_t num, size_t size) {
	return malloc(num * size);
}

static inline void free(void* ptr) {
	void* process = Process_getCurrent();
	void* handle = Process_getHeapHandle(process);
	MKDS_Free(handle, ptr);
}
