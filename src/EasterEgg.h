/**********************************************************************

  Sneedacity: A Digital Audio Editor

  EasterEgg.h

  moxniso

*******************************************************************//**

\class EasterEggThread

\brief wxThread inheritor for "SNED" easter egg

*//*******************************************************************/

#ifdef _WIN32

#pragma once
#include <wx/thread.h>
#include <wx/utils.h>

extern bool gEasterEggDone,
			gEasterEggMutex;

class EasterEgg {
public:
	EasterEgg();
	~EasterEgg();
	void CheckIfSneed();

	int mEasterEggStagesCompleted;
};

class EasterEggStartChecking : public wxThread {
public:
	EasterEggStartChecking() = default;
	~EasterEggStartChecking() = default;

	void *Entry()
	{
		EasterEgg once;
		while (!gEasterEggDone)
			once.CheckIfSneed();
		
		gEasterEggMutex = false;
		gEasterEggDone = false;
		return NULL;
	}

};

#endif // _WIN32
