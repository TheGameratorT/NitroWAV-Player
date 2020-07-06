.global SND_SetChannelTimer
SND_SetChannelTimer:
	STMFD   SP!, {R3,LR}
	MOV     R2, R1
	MOV     R3, #0
	MOV     R1, R0
	MOV     R0, #0x13
	STR     R3, [SP]
	BL      PushCommand_impl
	LDMFD   SP!, {R3,PC}