#pragma once

#include "Jukebox.h"

// Launchnik Rec'n'Run, a Player that will record notes into patterns and play
// them back. For now it only passes on the notes it receives and keeps the
// lights of its panel up to date; vocabulary follows CONTEXT.md.
class CRecNRun {
public:
	CRecNRun();

	void RenderBatch(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount);

private:
	void LoadEverything();
	void HandleDiff(const TJBox_PropertyDiff& iDiff);
	void UpdateLights();
	bool CanRun() const;

	TJBox_ObjectRef fCustomProperties;
	TJBox_ObjectRef fTransport;
	TJBox_ObjectRef fEnvironment;
	TJBox_ObjectRef fNoteStates;

	bool fLoaded = false;
	bool fOn = true;
	bool fRunning = false;
	bool fRec = false;
	bool fTransportPlaying = false;
	bool fBypassed = false;
	TJBox_Int32 fPatternIndex = 0;

	int fPatternLightFrame = -1;
	int fRunLightFrame = -1;
	int fRecLightFrame = -1;
};
