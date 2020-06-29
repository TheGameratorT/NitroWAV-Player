extern "C"
{
	void OS_Panic();
	void OS_WakeupThreadDirect(OSThread *thread);
	void OS_CreateThread(OSThread *thread, void (*func) (void *), void *arg, void *stack, u32 stackSize, u32 prio);
	bool OS_ReceiveMessage(OSMessageQueue *mq, OSMessage *msg, s32 flags);
	bool OS_SendMessage(OSMessageQueue *mq, OSMessage msg, s32 flags);
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
	
	bool FS_SeekFile(FSFile *p_file, s32 offset, FSSeekFileMode origin);
	s32  FS_ReadFile(FSFile *p_file, void *dst, s32 len);
	bool FS_CloseFile(FSFile *p_file);
	bool FS_OpenFileDirect(FSFile *p_file, FSArchive *p_arc, u32 image_top, u32 image_bottom, u32 file_index);
	void FS_InitFile(FSFile *p_file);
}

int getPlayerCount();