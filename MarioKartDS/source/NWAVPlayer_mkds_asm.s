@Hook initialization
ansub_0210D8A8:
	BL      NNS_SndInit
	BL      NWAV_Init
	LDR     R0, =NWAV_MainEventHandler
	BL      NWAV_SetEventHandler
	B       0x0210D8AC

@Hook fade update
ansub_0210D7C8:
	BL      NNS_SndMain
	BL      NWAV_UpdatePerFrame
	B       0x0210D7CC

@Hook play seq
ansub_0210D7DC:
	B       NWAVh_Play

@Hook stop seq
arepl_0210DA14:
arepl_0210DA7C:
arepl_0210DBB4:
	B       NWAVh_Stop
	
@Hook move seq volume
arepl_0210DAEC:
arepl_0210DBEC:
	B       NWAVh_MoveVolume

arepl_02106828:
arepl_02106CE8:
arepl_02106E30:
arepl_0210FB3C:
	B       NWAVh_SetTempo

arepl_0210FE90:
	B       NWAVh_SetPitch