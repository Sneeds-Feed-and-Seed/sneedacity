/**********************************************************************

  Sneedacity: A Digital Audio Editor

  ApplicationPrefs.h

  Anton Gerasimov

**********************************************************************/

#ifndef __SNEEDACITY_APPLICATION_PREFS__
#define __SNEEDACITY_APPLICATION_PREFS__

#include <wx/defs.h>

#include "PrefsPanel.h"
#include "Prefs.h"

class ShuttleGui;

class ApplicationPrefs final : public PrefsPanel
{
 public:
   ApplicationPrefs(wxWindow * parent, wxWindowID winid);
   ~ApplicationPrefs();
   ComponentInterfaceSymbol GetSymbol() override;
   TranslatableString GetDescription() override;

   bool Commit() override;
   ManualPageID HelpPageName() override;

 private:
   void Populate();
   void PopulateOrExchange(ShuttleGui & S) override;
};

#endif
