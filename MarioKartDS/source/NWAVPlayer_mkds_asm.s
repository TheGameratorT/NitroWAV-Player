@Hook initialization
ansub_0210D8A8:
	BL      NNS_SndInit
	BL      NWAV_Init
	LDR     R0, =NWAVi_MainEventHandler
	BL      NWAV_SetEventHandler
	B       0x0210D8AC

@Hook fade update
ansub_0210D7C8:
	BL      NNS_SndMain
	BL      NWAV_UpdateFade
	BL      NWAVi_UpdateMute
	B       0x0210D7CC

.global NWAVi_PlaySeq
NWAVi_PlaySeq:
	STMFD   SP!, {R4-R6,LR}
	B       0x0210D7E0
	
@Hook play seq
ansub_0210D7DC:
	MOV     R3, #1
	B       NWAVh_Play
	
@Hook play special seq
arepl_0210D644:
	MOV     R3, #0
	B       NWAVh_Play
	
@Hook mute seq
arepl_02106798:
arepl_021067E4:
@arepl_02106B60:
arepl_02106C94:
arepl_02106DDC:
arepl_02106EC4:
arepl_0210FB30:
	B       NWAVh_SetMute

@Hook stop seq
@arepl_02106DE8:
arepl_0210DA14:
arepl_0210DA7C:
arepl_0210DBB4:
	B       NWAVh_Stop
	
@Hook move seq volume
arepl_0210DAEC:
arepl_0210DBEC:
	B       NWAVh_MoveVolume

@Hook set seq volume
arepl_0210FF2C: @Pitch volume
	B       NWAVh_SetVolume

arepl_02106828:
arepl_02106CE8:
arepl_02106E30:
arepl_0210FB3C:
	B       NWAVh_SetTempo

arepl_0210FE90:
	B       NWAVh_SetPitch

.global NWAVi_LoadSeq
NWAVi_LoadSeq:
	STMFD   SP!, {R0-R3}
	B       0x0210E340

@Hook seq load
@arepl_020488F8:
ansub_0210E33C:
	B       NWAVh_LoadSeq
