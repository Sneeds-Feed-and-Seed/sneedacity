/**********************************************************************

  Sneedacity: A Digital Audio Editor

  EasterEgg.h

  moxniso

*******************************************************************//**

*//*******************************************************************/

#if defined(_WIN32) || defined(WIN32)

#include <wx/thread.h>
#include <wx/utils.h>

extern bool gEasterEggDone;
extern bool gEasterEggMutex;

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
