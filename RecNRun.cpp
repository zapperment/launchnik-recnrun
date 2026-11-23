#include "RecNRun.h"
#include "Tags.h"

namespace {

const TJBox_Float64 kPPQPerCrotchet = 15360.0;
const int kBatchFrames = 64;
const int kBarsPerPattern = 4;
// A play position further than this from where the previous batch ended is a
// jump (a semiquaver, as in Launchnik).
const TJBox_Float64 kJumpTolerance = 3840.0;

// Writes "pattern_<n>_part_<k>" (n and k from 1) to oName.
void MakePartName(int iPattern, int iPart, char oName[]) {
	const char prefix[] = "pattern_";
	const char middle[] = "_part_";
	int at = 0;
	for (int i = 0; prefix[i] != 0; ++i) {
		oName[at++] = prefix[i];
	}
	oName[at++] = static_cast<char>('0' + iPattern);
	for (int i = 0; middle[i] != 0; ++i) {
		oName[at++] = middle[i];
	}
	oName[at++] = static_cast<char>('0' + iPart);
	oName[at] = 0;
}

int ClampFrame(TJBox_Float64 iFrame) {
	if (iFrame < 0.0) {
		return 0;
	}
	if (iFrame > kBatchFrames - 1) {
		return kBatchFrames - 1;
	}
	return static_cast<int>(iFrame);
}

} // namespace

CRecNRun::CRecNRun(TJBox_Float64 iSampleRate) : fSampleRate(iSampleRate) {
	fCustomProperties = JBox_GetMotherboardObjectRef("/custom_properties");
	fTransport = JBox_GetMotherboardObjectRef("/transport");
	fEnvironment = JBox_GetMotherboardObjectRef("/environment");
	fNoteStates = JBox_GetMotherboardObjectRef("/note_states");
	char path[] = "/patterns/0";
	for (int pattern = 0; pattern < kPatternCount; ++pattern) {
		path[10] = static_cast<char>('0' + pattern);
		fPatternObjects[pattern] = JBox_GetMotherboardObjectRef(path);
		for (int part = 0; part < kPartCount; ++part) {
			char name[32];
			MakePartName(pattern + 1, part + 1, name);
			fPartProperties[pattern][part] = JBox_MakePropertyRef(fCustomProperties, name);
		}
	}
	// 30 ms, to stay clear of the host's 25.
	fBatchesBetweenWrites = static_cast<int>(0.03 * fSampleRate / kBatchFrames) + 1;
	for (int pattern = 0; pattern < kPatternCount; ++pattern) {
		for (int part = 0; part < kPartCount; ++part) {
			fBatchesSinceWrite[pattern][part] = fBatchesBetweenWrites;
		}
	}
}

void CRecNRun::RenderBatch(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount) {
	fIncomingCount = 0;
	fOutputCount = 0;
	// The first batch brings the starting value of every property as a
	// change, among them the Run request counter and RUN, which are not real
	// requests or presses. So the state is loaded instead, and only the notes
	// are taken.
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
	Render();
	FlushOutput();
	PublishPatterns();
	UpdateUnkept();
	UpdatePatternLengths();
	UpdateLights();
}

void CRecNRun::LoadEverything() {
	fOn = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(fCustomProperties, kOnTag));
	fRec = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(fCustomProperties, kRecTag));
	fBypassed = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(fEnvironment, kJBox_EnvironmentPlayerBypassed));
	fTransportPlaying = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(fTransport, kJBox_TransportPlaying));
	fRunning = CanRun() && fTransportPlaying;
	fPatternIndex = static_cast<TJBox_Int32>(JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportPatternIndex));
	for (int pattern = 0; pattern < kPatternCount; ++pattern) {
		LoadKeptPattern(pattern, JBox_LoadMOMPropertyByTag(fCustomProperties, kFirstKeptPatternTag + pattern));
	}
}

void CRecNRun::HandleDiff(const TJBox_PropertyDiff& iDiff) {
	// Notes are taken in now and handled in Render(), in the order of their
	// frames. A player bypassed by the host receives no notes at all.
	if (iDiff.fObjectRef == fNoteStates) {
		if (fIncomingCount < kMaxIncoming) {
			fIncoming[fIncomingCount++] = JBox_AsNoteEvent(&iDiff);
		}
		return;
	}
	if (iDiff.fObjectRef == fCustomProperties) {
		const TJBox_Tag tag = iDiff.fPropertyTag;
		// Switched off, the player stops; switched on again, it runs if
		// Reason is playing.
		if (tag == kOnTag) {
			fOn = JBox_GetBoolean(iDiff.fCurrentValue);
			fRunning = CanRun() && fTransportPlaying;
		}
		// REC keeps its state while the device is off or bypassed.
		else if (tag == kRecTag) {
			fRec = JBox_GetBoolean(iDiff.fCurrentValue);
		}
		// Each press of RUN starts or pauses the player on its own.
		else if (tag == kRunTag) {
			if (CanRun()) {
				fRunning = !fRunning;
			}
		}
		else if (tag >= kFirstKeptPatternTag && tag < kFirstKeptPatternTag + kPatternCount) {
			LoadKeptPattern(static_cast<int>(tag - kFirstKeptPatternTag), iDiff.fCurrentValue);
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

void CRecNRun::LoadKeptPattern(int iPattern, TJBox_Value iValue) {
	int length = static_cast<int>(JBox_GetStringLength(iValue));
	if (length > CPattern::kMaxTextLength) {
		length = CPattern::kMaxTextLength;
	}
	JBox_GetSubstring(iValue, 0, static_cast<TJBox_SizeT>(length), fScratch);
	CPattern& pattern = fPatterns[iPattern];

	// A keep: what was copied is what this pattern held when the panel last
	// saw it, so it holds no less now. The parts may lag a little behind, so
	// the copy may be shorter than the pattern, leaving the rest unkept.
	if (fLoaded && length >= fKeptLength[iPattern] && pattern.StartsWith(fScratch, length)) {
		fKeptLength[iPattern] = length;
		return;
	}

	// An undo, or a patch being loaded (or the song being opened): the
	// pattern becomes what is kept, and notes not yet kept are dropped,
	// along with those being recorded into it.
	if (iPattern == fRecordingPattern) {
		DropRecordings();
	}
	if (iPattern == fPreviousPattern) {
		EndPatternNotes();
	}
	pattern.Load(fScratch, length);
	fKeptLength[iPattern] = pattern.TextLength();
	for (int part = 0; part < kPartCount; ++part) {
		fPartDirty[iPattern][part] = true;
	}
}

void CRecNRun::Render() {
	const TJBox_Float64 tempo = JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportTempo);
	const TJBox_Float64 ticksPerFrame = (tempo / 60.0) * kPPQPerCrotchet / fSampleRate;
	const TJBox_Float64 batchTicks = kBatchFrames * ticksPerFrame;
	const TJBox_Float64 playPos = JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportPlayPos);
	const TJBox_Float64 patternStart = JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportPatternStartPos);
	const bool followingTransport = fTransportPlaying;
	const int pattern = SelectedPattern();
	fLength = PatternLength();

	// Playback carries on from the previous batch unless the device has just
	// started running, Reason's transport started or jumped, or the pattern
	// or its pattern clip changed. Otherwise the pattern's notes end, and so
	// does recording of notes still held.
	bool carryOn = fRunning && fWasRunning && followingTransport == fWasFollowingTransport &&
		pattern == fPreviousPattern && patternStart == fPreviousPatternStart;
	if (carryOn && followingTransport) {
		const TJBox_Float64 distance = playPos - (fPreviousPlayPos + fPreviousBatchTicks);
		carryOn = distance <= kJumpTolerance && distance >= -kJumpTolerance;
	}
	if (!carryOn) {
		EndPatternNotes();
		FinishRecordings();
	}
	if (!IsRecording()) {
		FinishRecordings();
	}

	// Started with RUN while Reason is stopped, the pattern plays from its
	// first bar.
	if (fRunning && !followingTransport && (!fWasRunning || fWasFollowingTransport)) {
		fOwnPosition = 0.0;
	}
	fPosition = followingTransport ? playPos - patternStart : fOwnPosition;

	// Incoming notes in the order of their frames (insertion sort keeps the
	// order of notes on the same frame).
	for (int i = 1; i < fIncomingCount; ++i) {
		const TJBox_NoteEvent note = fIncoming[i];
		int j = i - 1;
		while (j >= 0 && fIncoming[j].fAtFrameIndex > note.fAtFrameIndex) {
			fIncoming[j + 1] = fIncoming[j];
			--j;
		}
		fIncoming[j + 1] = note;
	}
	for (int i = 0; i < fIncomingCount; ++i) {
		HandleIncomingNote(fIncoming[i], ticksPerFrame);
	}

	if (fRunning && pattern >= 0 && fLength > 0) {
		PlayPattern(ticksPerFrame);
	}

	fElapsed += batchTicks;
	if (fRunning && !followingTransport) {
		fOwnPosition += batchTicks;
	}
	fWasRunning = fRunning;
	fWasFollowingTransport = followingTransport;
	fPreviousPlayPos = playPos;
	fPreviousBatchTicks = batchTicks;
	fPreviousPatternStart = patternStart;
	fPreviousPattern = pattern;
}

void CRecNRun::HandleIncomingNote(const TJBox_NoteEvent& iNote, TJBox_Float64 iTicksPerFrame) {
	const int noteNumber = iNote.fNoteNumber;
	const int frame = iNote.fAtFrameIndex;
	const TJBox_Float64 offset = frame * iTicksPerFrame;
	const TJBox_Float64 time = fElapsed + offset;

	if (iNote.fVelocity == 0) {
		Output(iNote.fNoteNumber, 0, frame);
		fHeld[noteNumber] = false;
		if (fRecordingNote[noteNumber]) {
			FinishRecording(noteNumber, time);
		}
		return;
	}

	// A played note wins over the same note of the pattern.
	if (fPatternSounding[noteNumber]) {
		Output(iNote.fNoteNumber, 0, frame);
		fPatternSounding[noteNumber] = false;
	}
	if (fRecordingNote[noteNumber]) {
		FinishRecording(noteNumber, time);
	}
	Output(iNote.fNoteNumber, iNote.fVelocity, frame);
	fHeld[noteNumber] = true;

	const int pattern = SelectedPattern();
	if (!IsRecording() || fPatterns[pattern].IsFull()) {
		return;
	}
	if (fRecordingPattern != pattern) {
		FinishRecordings();
		fRecordingPattern = pattern;
	}
	TJBox_Int32 start = static_cast<TJBox_Int32>(PatternPosition(offset));
	if (start >= fLength) {
		start = fLength - 1;
	}
	fRecordingNote[noteNumber] = true;
	fRecordingStart[noteNumber] = start;
	fRecordingStartTime[noteNumber] = time;
	fRecordingVelocity[noteNumber] = iNote.fVelocity;
}

void CRecNRun::PlayPattern(TJBox_Float64 iTicksPerFrame) {
	const CPattern& pattern = fPatterns[SelectedPattern()];
	const TJBox_Float64 batchTicks = kBatchFrames * iTicksPerFrame;
	const TJBox_Float64 from = PatternPosition(0.0);

	// Notes starting in this batch, which covers the pattern from `from` on,
	// wrapping round at its end.
	for (int i = 0; i < pattern.NoteCount(); ++i) {
		const TRecordedNote& note = pattern.Note(i);
		// After the time signature changed, notes beyond the end do not play.
		if (note.fStart >= fLength) {
			continue;
		}
		TJBox_Float64 offset = note.fStart - from;
		if (offset < 0.0) {
			offset += fLength;
		}
		if (offset >= batchTicks) {
			continue;
		}
		const int noteNumber = note.fNoteNumber;
		// A note held on the controller wins.
		if (fHeld[noteNumber]) {
			continue;
		}
		const int frame = ClampFrame(offset / iTicksPerFrame);
		if (fPatternSounding[noteNumber]) {
			Output(note.fNoteNumber, 0, frame);
		}
		Output(note.fNoteNumber, note.fVelocity, frame);
		fPatternSounding[noteNumber] = true;
		fPatternNoteEnd[noteNumber] = fElapsed + offset + note.fLength;
	}

	// Notes ending in this batch.
	for (int noteNumber = 0; noteNumber < 128; ++noteNumber) {
		if (fPatternSounding[noteNumber] && fPatternNoteEnd[noteNumber] < fElapsed + batchTicks) {
			Output(static_cast<TJBox_UInt8>(noteNumber), 0, ClampFrame((fPatternNoteEnd[noteNumber] - fElapsed) / iTicksPerFrame));
			fPatternSounding[noteNumber] = false;
		}
	}
}

void CRecNRun::EndPatternNotes() {
	for (int noteNumber = 0; noteNumber < 128; ++noteNumber) {
		if (fPatternSounding[noteNumber]) {
			Output(static_cast<TJBox_UInt8>(noteNumber), 0, 0);
			fPatternSounding[noteNumber] = false;
		}
	}
}

// A note being recorded ends at iEnd (fElapsed), or at the end of the
// pattern's fourth bar if that comes first.
void CRecNRun::FinishRecording(int iNoteNumber, TJBox_Float64 iEnd) {
	fRecordingNote[iNoteNumber] = false;
	if (fRecordingPattern < 0 || fRecordingPattern >= kPatternCount) {
		return;
	}
	const TJBox_Int32 start = fRecordingStart[iNoteNumber];
	TJBox_Int32 length = static_cast<TJBox_Int32>(iEnd - fRecordingStartTime[iNoteNumber] + 0.5);
	if (length < 1) {
		length = 1;
	}
	if (length > fLength - start) {
		length = fLength - start;
	}
	if (length < 1) {
		return;
	}
	CPattern& pattern = fPatterns[fRecordingPattern];
	const int textLength = pattern.TextLength();
	TRecordedNote note;
	note.fStart = start;
	note.fLength = length;
	note.fNoteNumber = static_cast<TJBox_UInt8>(iNoteNumber);
	note.fVelocity = fRecordingVelocity[iNoteNumber];
	if (pattern.Add(note)) {
		MarkChanged(fRecordingPattern, textLength);
	}
}

// Notes still held when recording ends end now.
void CRecNRun::FinishRecordings() {
	for (int noteNumber = 0; noteNumber < 128; ++noteNumber) {
		if (fRecordingNote[noteNumber]) {
			FinishRecording(noteNumber, fElapsed);
		}
	}
}

void CRecNRun::DropRecordings() {
	for (int noteNumber = 0; noteNumber < 128; ++noteNumber) {
		fRecordingNote[noteNumber] = false;
	}
}

// The pattern's text changed from character iFromCharacter on.
void CRecNRun::MarkChanged(int iPattern, int iFromCharacter) {
	const int last = (fPatterns[iPattern].TextLength() - 1) / kPartSize;
	for (int part = iFromCharacter / kPartSize; part <= last && part < kPartCount; ++part) {
		fPartDirty[iPattern][part] = true;
	}
}

void CRecNRun::Output(TJBox_UInt8 iNoteNumber, TJBox_UInt8 iVelocity, int iFrame) {
	if (fOutputCount >= kMaxOutput) {
		return;
	}
	TJBox_NoteEvent& event = fOutput[fOutputCount++];
	event.fNoteNumber = iNoteNumber;
	event.fVelocity = iVelocity;
	event.fAtFrameIndex = static_cast<TJBox_UInt16>(iFrame);
}

// Sends the notes in the order of their frames; notes on the same frame keep
// the order they were made in, so a note ended and started again stays so.
void CRecNRun::FlushOutput() {
	for (int i = 1; i < fOutputCount; ++i) {
		const TJBox_NoteEvent event = fOutput[i];
		int j = i - 1;
		while (j >= 0 && fOutput[j].fAtFrameIndex > event.fAtFrameIndex) {
			fOutput[j + 1] = fOutput[j];
			--j;
		}
		fOutput[j + 1] = event;
	}
	for (int i = 0; i < fOutputCount; ++i) {
		JBox_OutputNoteEvent(fOutput[i]);
	}
}

void CRecNRun::PublishPatterns() {
	for (int pattern = 0; pattern < kPatternCount; ++pattern) {
		const CPattern& held = fPatterns[pattern];
		for (int part = 0; part < kPartCount; ++part) {
			if (fBatchesSinceWrite[pattern][part] < fBatchesBetweenWrites) {
				++fBatchesSinceWrite[pattern][part];
			}
			if (!fPartDirty[pattern][part] || fBatchesSinceWrite[pattern][part] < fBatchesBetweenWrites) {
				continue;
			}
			const int start = part * kPartSize;
			int size = held.TextLength() - start;
			if (size < 0) {
				size = 0;
			}
			if (size > kPartSize) {
				size = kPartSize;
			}
			const char* text = held.Text() + (size > 0 ? start : 0);
			JBox_SetRTStringData(fPartProperties[pattern][part], static_cast<TJBox_SizeT>(size), reinterpret_cast<const TJBox_UInt8*>(text));
			fPartDirty[pattern][part] = false;
			fBatchesSinceWrite[pattern][part] = 0;
		}
	}
}

// Tells the panel which patterns have notes not yet kept, so that a click
// copies only those.
void CRecNRun::UpdateUnkept() {
	int bits = 0;
	for (int pattern = 0; pattern < kPatternCount; ++pattern) {
		if (fKeptLength[pattern] != fPatterns[pattern].TextLength()) {
			bits |= 1 << pattern;
		}
	}
	if (bits != fUnkeptBits) {
		fUnkeptBits = bits;
		JBox_StoreMOMPropertyAsNumber(fCustomProperties, kUnkeptTag, bits);
	}
}

// Tells Reason how long the patterns are, for the pattern clips.
void CRecNRun::UpdatePatternLengths() {
	if (fLength == fWrittenPatternLength || fLength <= 0) {
		return;
	}
	fWrittenPatternLength = fLength;
	for (int pattern = 0; pattern < kPatternCount; ++pattern) {
		JBox_StoreMOMPropertyAsNumber(fPatternObjects[pattern], kJBox_PatternsLength, fLength);
	}
}

bool CRecNRun::CanRun() const {
	return fOn && !fBypassed;
}

bool CRecNRun::IsRecording() const {
	return fRunning && fRec && SelectedPattern() >= 0 && fLength > 0;
}

int CRecNRun::SelectedPattern() const {
	return (fPatternIndex >= 0 && fPatternIndex < kPatternCount) ? static_cast<int>(fPatternIndex) : -1;
}

// Four bars of Reason's time signature, a bar being as many of its beats as
// the numerator says.
TJBox_Int32 CRecNRun::PatternLength() const {
	int numerator = static_cast<int>(JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportTimeSignatureNumerator));
	int denominator = static_cast<int>(JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportTimeSignatureDenominator));
	if (numerator <= 0 || denominator <= 0) {
		numerator = 4;
		denominator = 4;
	}
	const TJBox_Int32 beat = static_cast<TJBox_Int32>(kPPQPerCrotchet) * 4 / denominator;
	return kBarsPerPattern * numerator * beat;
}

// Where in the pattern the current batch is, iOffset ticks after its start.
TJBox_Float64 CRecNRun::PatternPosition(TJBox_Float64 iOffset) const {
	if (fLength <= 0) {
		return 0.0;
	}
	const TJBox_Float64 length = static_cast<TJBox_Float64>(fLength);
	TJBox_Float64 position = fPosition + iOffset;
	position -= length * static_cast<TJBox_Float64>(static_cast<long long>(position / length));
	if (position < 0.0) {
		position += length;
	}
	return position;
}

void CRecNRun::UpdateLights() {
	// Frame 0 of the pattern buttons has none lit, frames 1 to 8 one each.
	const int patternLightFrame = SelectedPattern() + 1;
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
