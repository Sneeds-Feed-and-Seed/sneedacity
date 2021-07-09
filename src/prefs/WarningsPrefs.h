/**********************************************************************

  Sneedacity: A Digital Audio Editor

  WarningsPrefs.h

  Brian Gunlogson
  Joshua Haberman
  James Crook

**********************************************************************/

#ifndef __SNEEDACITY_WARNINGS_PREFS__
#define __SNEEDACITY_WARNINGS_PREFS__

#include <wx/defs.h>

#include "PrefsPanel.h"

class ShuttleGui;

#define WARNINGS_PREFS_PLUGIN_SYMBOL ComponentInterfaceSymbol{ XO("Warnings") }

class WarningsPrefs final : public PrefsPanel
{
 public:
   WarningsPrefs(wxWindow * parent, wxWindowID winid);
   ~WarningsPrefs();
   ComponentInterfaceSymbol GetSymbol() override;
   TranslatableString GetDescription() override;

   bool Commit() override;
   ManualPageID HelpPageName() override;

 private:
   void Populate();
   void PopulateOrExchange(ShuttleGui & S) override;
};

#endif
