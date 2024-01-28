#include <nds.h>

#ifndef _OS_H
#define _OS_H

#define OS_MESSAGE_NOBLOCK 0
#define OS_MESSAGE_BLOCK 1

typedef u8 OSThread[0xC0];

struct _OSThreadQueue
{
    OSThread *head;
    OSThread *tail;
};

typedef void *OSMessage;
typedef u8 OSMessageQueue[32];

typedef struct _OSThreadQueue OSThreadQueue;

#ifdef __cplusplus
extern "C"
{
#endif

	void OS_Terminate();
	void OS_CreateThread(OSThread *thread, void (*func) (void *), void *arg, void *stack, u32 stackSize, u32 prio);
	void OS_DestroyThread(OSThread *thread);
	u64 OS_GetTick();
	void OS_InitMessageQueue(void *mq, void *msgArray, s32 msgCount);
	void OS_InitThread();
	bool OS_ReceiveMessage(void *mq, void *msg, s32 flags);
	bool OS_SendMessage(void *mq, void* msg, s32 flags);
	void OS_SleepThread(OSThreadQueue *queue);
	int OS_SPrintf(char *dst, const char *fmt, ...);
	void OS_WakeupThreadDirect(OSThread* thread);

	void DC_FlushRange(const void *startAddr, u32 nBytes);
	void DC_StoreRange(const void *startAddr, u32 nBytes);

#ifdef __cplusplus
}
#endif

#endif