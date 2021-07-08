/**********************************************************************

  Sneedacity: A Digital Audio Editor

  EasterEgg.cpp

  moxniso

*******************************************************************//**

\class EasterEgg

\brief An easter egg in Sneedacity whereby typing "sneed" after 
selecting a track will bring you to the /g/ catalog.

*//*******************************************************************/

#ifdef _WIN32

#include <Windows.h>
#include "EasterEgg.h"

// this char array must be declared as int or else GetAsyncKeyState won't work
const int gEasterEggSteps[] = {'S', 'N', 'E', 'E', 'D'};
bool gEasterEggDone = false,
	 gEasterEggMutex = false;

void EasterEgg::CheckIfSneed(void)
{
	// wxwidgets does not have an quick way of checking individual keyup/keydown states
	// so I'll have to use a win32 specific function here :/

	for (int i = 0;i <= 4;i++) {
		int current = gEasterEggSteps[i];
		if (GetAsyncKeyState(current) & 0x8000) {
			if (((current == 'S') && (mEasterEggStagesCompleted >= 1)) ||
				((current == 'N') && (mEasterEggStagesCompleted >= 2)) ||
			    ((current == 'E') && (mEasterEggStagesCompleted >= 4)))
					break;
			
			this->mEasterEggStagesCompleted++;
		}
	}
			
	if (this->mEasterEggStagesCompleted >= 5) 
	{
		if (!gEasterEggDone) // to stop multiple tabs from appearing
			wxLaunchDefaultBrowser((wxString)"https://4chan.org/g/sneed");
		
		gEasterEggDone = true;
	}
}

EasterEgg::EasterEgg() 
{
	this->mEasterEggStagesCompleted = 0;
}

EasterEgg::~EasterEgg()
{
}

#endif // _WIN32
