#pragma once

#include "Jukebox.h"
#include "Pattern.h"

// Launchnik Rec'n'Run, a Player that records notes into eight patterns and
// plays them back; vocabulary follows CONTEXT.md.
//
// The realtime code cannot write anything that is saved, so recorded notes
// live here until a click on the panel keeps them: each pattern is shown, as
// text, in realtime strings that the click copies into the kept pattern
// strings (display.lua). A change of a kept pattern comes back here. If it is
// the start of what this pattern holds, at least as long as the last one, it
// is a keep; otherwise it is an undo or a patch being loaded, and the pattern
// is loaded from it, dropping whatever was not yet kept.
class CRecNRun {
public:
	explicit CRecNRun(TJBox_Float64 iSampleRate);

	void RenderBatch(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount);

	static const int kPatternCount = 8;
	// Parts of the realtime strings showing a pattern, of kPartSize characters.
	static const int kPartCount = 5;
	static const int kPartSize = 2048;

private:
	void LoadEverything();
	void HandleDiff(const TJBox_PropertyDiff& iDiff);
	void LoadKeptPattern(int iPattern, TJBox_Value iValue);
	void Render();
	void HandleIncomingNote(const TJBox_NoteEvent& iNote, TJBox_Float64 iTicksPerFrame);
	void PlayPattern(TJBox_Float64 iTicksPerFrame);
	void EndPatternNotes();
	void FinishRecording(int iNoteNumber, TJBox_Float64 iEnd);
	void FinishRecordings();
	void DropRecordings();
	void MarkChanged(int iPattern, int iFromCharacter);
	void Output(TJBox_UInt8 iNoteNumber, TJBox_UInt8 iVelocity, int iFrame);
	void FlushOutput();
	void PublishPatterns();
	void UpdateUnkept();
	void UpdatePatternLengths();
	void UpdateLights();
	bool CanRun() const;
	bool IsRecording() const;
	int SelectedPattern() const;
	TJBox_Int32 PatternLength() const;
	TJBox_Float64 PatternPosition(TJBox_Float64 iOffset) const;

	TJBox_ObjectRef fCustomProperties;
	TJBox_ObjectRef fTransport;
	TJBox_ObjectRef fEnvironment;
	TJBox_ObjectRef fNoteStates;
	TJBox_ObjectRef fPatternObjects[kPatternCount];
	TJBox_PropertyRef fPartProperties[kPatternCount][kPartCount];

	const TJBox_Float64 fSampleRate;

	bool fLoaded = false;
	bool fOn = true;
	bool fRunning = false;
	bool fRec = false;
	bool fTransportPlaying = false;
	bool fBypassed = false;
	TJBox_Int32 fPatternIndex = 0;

	// The patterns as held here, and how much of each one's text is kept.
	CPattern fPatterns[kPatternCount];
	int fKeptLength[kPatternCount] = {};
	// Parts to be written to the realtime strings, and how many batches ago
	// each was last written: the host takes at most one write per 25 ms.
	bool fPartDirty[kPatternCount][kPartCount] = {};
	int fBatchesSinceWrite[kPatternCount][kPartCount] = {};
	int fBatchesBetweenWrites = 1;

	// Playback, as of the end of the previous batch. fElapsed counts ticks
	// (15360 PPQ) since the device was created and never jumps; it times the
	// ends of notes.
	TJBox_Float64 fElapsed = 0.0;
	bool fWasRunning = false;
	bool fWasFollowingTransport = false;
	TJBox_Float64 fPreviousPlayPos = 0.0;
	TJBox_Float64 fPreviousBatchTicks = 0.0;
	TJBox_Float64 fPreviousPatternStart = 0.0;
	int fPreviousPattern = -1;
	// While running without Reason's transport: the position in the pattern
	// at the start of this batch. Otherwise it follows the play position.
	TJBox_Float64 fOwnPosition = 0.0;
	TJBox_Float64 fPosition = 0.0;
	TJBox_Int32 fLength = 0;

	// Notes of the pattern that are sounding, with when they end (fElapsed).
	bool fPatternSounding[128] = {};
	TJBox_Float64 fPatternNoteEnd[128] = {};
	// Notes held on the controller.
	bool fHeld[128] = {};
	// Notes being recorded: where they started in the pattern, when
	// (fElapsed), and how hard; all into fRecordingPattern.
	bool fRecordingNote[128] = {};
	TJBox_Int32 fRecordingStart[128] = {};
	TJBox_Float64 fRecordingStartTime[128] = {};
	TJBox_UInt8 fRecordingVelocity[128] = {};
	int fRecordingPattern = -1;

	// Notes received in this batch, and the notes to send, put in the order
	// of their frames before they go.
	static const int kMaxIncoming = 256;
	TJBox_NoteEvent fIncoming[kMaxIncoming];
	int fIncomingCount = 0;
	static const int kMaxOutput = 1024;
	TJBox_NoteEvent fOutput[kMaxOutput];
	int fOutputCount = 0;

	char fScratch[CPattern::kMaxTextLength + 1] = {};

	int fPatternLightFrame = -1;
	int fRunLightFrame = -1;
	int fRecLightFrame = -1;
	int fUnkeptBits = -1;
	TJBox_Int32 fWrittenPatternLength = -1;
};
