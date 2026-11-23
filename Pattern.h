#pragma once

#include "Jukebox.h"

// One recorded note. Positions and lengths are in 15360 PPQ, counted from the
// first beat of the pattern.
struct TRecordedNote {
	TJBox_Int32 fStart;
	TJBox_Int32 fLength;
	TJBox_UInt8 fNoteNumber;
	TJBox_UInt8 fVelocity;
};

// A pattern as the realtime code holds it: its notes in the order they were
// recorded, and the same notes as text, which is how the pattern is kept.
//
// The text is empty for a pattern without notes. Otherwise it is the format
// version "1" followed by ten characters per note, each carrying six bits
// (A-Z, a-z, 0-9, "-", "_"), most significant first, of
//   start (22 bits) | length (22 bits) | note number (7 bits) | velocity (7 bits)
// with two unused bits on top. A note is only ever added at the end, so an
// earlier text of the same pattern is always the start of a later one.
class CPattern {
public:
	static const int kMaxNotes = 1000;
	static const int kNoteTextLength = 10;
	static const int kMaxTextLength = 1 + kMaxNotes * kNoteTextLength;
	// The largest start or length the text can carry, a good deal more than
	// four bars of 16/2.
	static const TJBox_Int32 kMaxPosition = (1 << 22) - 1;

	void Clear();

	// Replaces the notes with those of a kept text. Notes that cannot be read
	// are left out; a text of an unknown version gives an empty pattern.
	void Load(const char iText[], int iLength);

	// Adds a note at the end, unless the pattern is full.
	bool Add(const TRecordedNote& iNote);

	bool IsFull() const { return fNoteCount >= kMaxNotes; }
	int NoteCount() const { return fNoteCount; }
	const TRecordedNote& Note(int iIndex) const { return fNotes[iIndex]; }
	const char* Text() const { return fText; }
	int TextLength() const { return fTextLength; }

	// Whether iText is the start of this pattern's text (or all of it).
	bool StartsWith(const char iText[], int iLength) const;

private:
	TRecordedNote fNotes[kMaxNotes];
	int fNoteCount = 0;
	char fText[kMaxTextLength + 1] = {};
	int fTextLength = 0;
};
