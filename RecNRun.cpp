#include "RecNRun.h"
#include "Tags.h"

CRecNRun::CRecNRun() {
	fCustomProperties = JBox_GetMotherboardObjectRef("/custom_properties");
	fTransport = JBox_GetMotherboardObjectRef("/transport");
	fEnvironment = JBox_GetMotherboardObjectRef("/environment");
	fNoteStates = JBox_GetMotherboardObjectRef("/note_states");
}

void CRecNRun::RenderBatch(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount) {
	// The first batch brings the starting value of every property as a
	// change, among them the Run request counter and RUN, which are not real
	// requests or presses. So the state is loaded instead, and only the notes
	// are passed on.
	if (!fLoaded) {
		LoadEverything();
		fLoaded = true;
		for (TJBox_UInt32 i = 0; i < iDiffCount; ++i) {
			if (iPropertyDiffs[i].fObjectRef == fNoteStates) {
				HandleDiff(iPropertyDiffs[i]);
			}
		}
	}
	else {
		for (TJBox_UInt32 i = 0; i < iDiffCount; ++i) {
			HandleDiff(iPropertyDiffs[i]);
		}
	}
	UpdateLights();
}

void CRecNRun::LoadEverything() {
	fOn = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(fCustomProperties, kOnTag));
	fRec = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(fCustomProperties, kRecTag));
	fBypassed = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(fEnvironment, kJBox_EnvironmentPlayerBypassed));
	fTransportPlaying = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(fTransport, kJBox_TransportPlaying));
	fRunning = CanRun() && fTransportPlaying;
	fPatternIndex = static_cast<TJBox_Int32>(JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportPatternIndex));
}

void CRecNRun::HandleDiff(const TJBox_PropertyDiff& iDiff) {
	// Every note is passed on unchanged, so that the instrument below can be
	// played while the device is in the rack. A player bypassed by the host
	// receives no notes at all.
	if (iDiff.fObjectRef == fNoteStates) {
		JBox_OutputNoteEvent(JBox_AsNoteEvent(&iDiff));
		return;
	}
	if (iDiff.fObjectRef == fCustomProperties) {
		// Switched off, the player stops; switched on again, it runs if
		// Reason is playing.
		if (iDiff.fPropertyTag == kOnTag) {
			fOn = JBox_GetBoolean(iDiff.fCurrentValue);
			fRunning = CanRun() && fTransportPlaying;
		}
		// REC keeps its state while the device is off or bypassed.
		else if (iDiff.fPropertyTag == kRecTag) {
			fRec = JBox_GetBoolean(iDiff.fCurrentValue);
		}
		// Each press of RUN starts or pauses the player on its own.
		else if (iDiff.fPropertyTag == kRunTag) {
			if (CanRun()) {
				fRunning = !fRunning;
			}
		}
		return;
	}
	// Bypassed, the player stops and ignores Run requests; no longer
	// bypassed, it runs if Reason is playing.
	if (iDiff.fObjectRef == fEnvironment && iDiff.fPropertyTag == kJBox_EnvironmentPlayerBypassed) {
		fBypassed = JBox_GetBoolean(iDiff.fCurrentValue);
		fRunning = CanRun() && fTransportPlaying;
		return;
	}
	if (iDiff.fObjectRef != fTransport) {
		return;
	}
	switch (iDiff.fPropertyTag) {
		case kJBox_TransportPatternIndex:
			fPatternIndex = static_cast<TJBox_Int32>(JBox_GetNumber(iDiff.fCurrentValue));
			break;
		// The player starts and stops with Reason's transport. Reason sends
		// no Run request when its transport starts, and a Stop request only
		// when stop is pressed while already stopped (seen in Recon).
		case kJBox_TransportPlaying:
			fTransportPlaying = JBox_GetBoolean(iDiff.fCurrentValue);
			fRunning = CanRun() && fTransportPlaying;
			break;
		// Run requests come, for instance, from the Combinator's Run Pattern
		// Devices button.
		case kJBox_TransportRequestRun:
			fRunning = CanRun();
			break;
		case kJBox_TransportRequestStop:
			fRunning = false;
			break;
	}
}

bool CRecNRun::CanRun() const {
	return fOn && !fBypassed;
}

void CRecNRun::UpdateLights() {
	// Frame 0 of the pattern buttons has none lit, frames 1 to 8 one each.
	const int patternLightFrame = (fPatternIndex >= 0 && fPatternIndex < 8) ? fPatternIndex + 1 : 0;
	if (patternLightFrame != fPatternLightFrame) {
		fPatternLightFrame = patternLightFrame;
		JBox_StoreMOMPropertyAsNumber(fCustomProperties, kPatternLightFrameTag, patternLightFrame);
	}

	// A player that is off or bypassed should look inactive.
	const int runLightFrame = !CanRun() ? kButtonFrameInactive : (fRunning ? kButtonFrameLit : kButtonFrameUnlit);
	if (runLightFrame != fRunLightFrame) {
		fRunLightFrame = runLightFrame;
		JBox_StoreMOMPropertyAsNumber(fCustomProperties, kRunLightFrameTag, runLightFrame);
	}

	const int recLightFrame = !CanRun() ? kButtonFrameInactive : (fRec ? kButtonFrameLit : kButtonFrameUnlit);
	if (recLightFrame != fRecLightFrame) {
		fRecLightFrame = recLightFrame;
		JBox_StoreMOMPropertyAsNumber(fCustomProperties, kRecLightFrameTag, recLightFrame);
	}
}
