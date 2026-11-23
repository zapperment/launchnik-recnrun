#include "Pattern.h"

namespace {

const char kFormatVersion = '1';

const char kDigits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// The value of one character of a note, or -1 if it is none of kDigits.
int DigitValue(char iCharacter) {
	if (iCharacter >= 'A' && iCharacter <= 'Z') {
		return iCharacter - 'A';
	}
	if (iCharacter >= 'a' && iCharacter <= 'z') {
		return iCharacter - 'a' + 26;
	}
	if (iCharacter >= '0' && iCharacter <= '9') {
		return iCharacter - '0' + 52;
	}
	if (iCharacter == '-') {
		return 62;
	}
	if (iCharacter == '_') {
		return 63;
	}
	return -1;
}

void EncodeNote(const TRecordedNote& iNote, char oText[]) {
	const TJBox_UInt64 bits =
		(static_cast<TJBox_UInt64>(iNote.fStart) << 36) |
		(static_cast<TJBox_UInt64>(iNote.fLength) << 14) |
		(static_cast<TJBox_UInt64>(iNote.fNoteNumber) << 7) |
		static_cast<TJBox_UInt64>(iNote.fVelocity);
	for (int i = 0; i < CPattern::kNoteTextLength; ++i) {
		oText[i] = kDigits[(bits >> (54 - 6 * i)) & 63];
	}
}

bool DecodeNote(const char iText[], TRecordedNote& oNote) {
	TJBox_UInt64 bits = 0;
	for (int i = 0; i < CPattern::kNoteTextLength; ++i) {
		const int digit = DigitValue(iText[i]);
		if (digit < 0) {
			return false;
		}
		bits = (bits << 6) | static_cast<TJBox_UInt64>(digit);
	}
	oNote.fStart = static_cast<TJBox_Int32>((bits >> 36) & CPattern::kMaxPosition);
	oNote.fLength = static_cast<TJBox_Int32>((bits >> 14) & CPattern::kMaxPosition);
	oNote.fNoteNumber = static_cast<TJBox_UInt8>((bits >> 7) & 127);
	oNote.fVelocity = static_cast<TJBox_UInt8>(bits & 127);
	return oNote.fLength > 0 && oNote.fVelocity > 0;
}

} // namespace

void CPattern::Clear() {
	fNoteCount = 0;
	fTextLength = 0;
	fText[0] = 0;
}

void CPattern::Load(const char iText[], int iLength) {
	Clear();
	if (iLength <= 0 || iText[0] != kFormatVersion) {
		return;
	}
	for (int at = 1; at + kNoteTextLength <= iLength; at += kNoteTextLength) {
		TRecordedNote note;
		if (DecodeNote(iText + at, note)) {
			Add(note);
		}
	}
}

bool CPattern::Add(const TRecordedNote& iNote) {
	if (IsFull() || iNote.fStart < 0 || iNote.fStart > kMaxPosition || iNote.fLength <= 0 ||
		iNote.fLength > kMaxPosition || iNote.fNoteNumber > 127 || iNote.fVelocity == 0 || iNote.fVelocity > 127) {
		return false;
	}
	fNotes[fNoteCount] = iNote;
	++fNoteCount;
	if (fTextLength == 0) {
		fText[0] = kFormatVersion;
		fTextLength = 1;
	}
	EncodeNote(iNote, fText + fTextLength);
	fTextLength += kNoteTextLength;
	fText[fTextLength] = 0;
	return true;
}

bool CPattern::StartsWith(const char iText[], int iLength) const {
	if (iLength > fTextLength) {
		return false;
	}
	for (int i = 0; i < iLength; ++i) {
		if (iText[i] != fText[i]) {
			return false;
		}
	}
	return true;
}
