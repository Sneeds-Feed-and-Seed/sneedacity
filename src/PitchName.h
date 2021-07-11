/**********************************************************************

   Sneedacity: A Digital Audio Editor
   Sneedacity is copyright (c) 1999-2013 Audacity Team,
                 copyright (c) 2021 Sneedacity Team.
   License: GPL v2.  See License.txt.

   PitchName.h
   Vaughan Johnson, Dominic Mazzoni

******************************************************************//**

   utilities for converting among frequency, MIDI note number,
   pitch index, pitch name

*//*******************************************************************/


#ifndef __SNEEDACITY_PITCHNAME__
#define __SNEEDACITY_PITCHNAME__

class TranslatableString;

// FreqToMIDInote takes a frequency in Hz (exponential scale relative to
// alphabetic pitch names) and returns a pitch ID number (linear
// scale), such that A440 (A4) is 69, middle C (C4) is 60, etc.
// Each register starts with C (e.g., for middle C and A440,
// it's register 4).
// MIDI note number 0 is C-1 in Scientific pitch notation.
SNEEDACITY_DLL_API double FreqToMIDInote(const double freq);

SNEEDACITY_DLL_API double MIDInoteToFreq(const double dMIDInote);

// PitchIndex returns the [0,11] index for a double MIDI note number,
// per result from FreqToMIDInote, corresponding to modulo 12
// of the integer part of (dMIDInote + 0.5), so 0=C, 1=C#, etc.
SNEEDACITY_DLL_API unsigned int PitchIndex(const double dMIDInote);

// PitchOctave returns the octave index for a double dMIDInote note number,
// per result from FreqToMIDInote.
// MIDI note number 0 is C-1 in Scientific pitch notation.
SNEEDACITY_DLL_API int PitchOctave(const double dMIDInote);

enum class PitchNameChoice { Sharps, Flats, Both };

// PitchName takes dMIDInote (per result from
// FreqToMIDInote) and returns a standard pitch/note name [C, C#, etc.).
SNEEDACITY_DLL_API TranslatableString PitchName(
   const double dMIDInote,
   const PitchNameChoice choice = PitchNameChoice::Sharps );

// PitchName_Absolute does the same thing as PitchName, but appends
// the octave number, e.g., instead of "C" it will return "C4"
// if the dMIDInote corresponds to middle C, i.e., is 60.
SNEEDACITY_DLL_API TranslatableString PitchName_Absolute(
   const double dMIDInote,
   const PitchNameChoice choice = PitchNameChoice::Sharps);

SNEEDACITY_DLL_API
double PitchToMIDInote(const unsigned int nPitchIndex, const int nPitchOctave);

SNEEDACITY_DLL_API
double PitchToFreq(const unsigned int nPitchIndex, const int nPitchOctave);

#endif	// __SNEEDACITY_PITCHNAME__
