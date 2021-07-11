/**********************************************************************

  Sneedacity: A Digital Audio Editor

  Catalog.cpp

  moxniso

*******************************************************************//**

\class Catalog

\brief An easter egg in Sneedacity whereby typing "sneed" after 
selecting a track will bring you to the /g/ catalog. As this is technically
telemetry, it is disabled by default, and must be enabled manually in Preferences.

*//*******************************************************************/

#include <wx/wx.h>
#include "Catalog.h"
#include "Prefs.h"

const int gCatalogSteps[] = {'S', 'N', 'E', 'E', 'D'};
int gCatalogStepsCompleted = 0;

void CatalogListener::CheckForSneed(wxKeyEvent& kev)
{
	bool enabled = false;
	gPrefs->Read(wxT("/GUI/Catalog"), &enabled);
	
	if (enabled) 
	{
		if (kev.GetKeyCode() == gCatalogSteps[gCatalogStepsCompleted])
			gCatalogStepsCompleted++;
		else
			gCatalogStepsCompleted = 0;
		
		if (gCatalogStepsCompleted >= 5) {
			gCatalogStepsCompleted = 0;
			wxLaunchDefaultBrowser((wxString)"https://4chan.org/g/sneed");
		}
	}

	kev.ResumePropagation(1);
	kev.Skip();
}

