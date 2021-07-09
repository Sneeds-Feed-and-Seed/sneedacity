/**********************************************************************

  Sneedacity: A Digital Audio Editor

  Catalog.h

  moxniso

*******************************************************************//**

*//*******************************************************************/

#pragma once

#include <wx/event.h>

class CatalogListener : public wxEvtHandler {
public:
	void CheckForSneed(wxKeyEvent& kev);
};
