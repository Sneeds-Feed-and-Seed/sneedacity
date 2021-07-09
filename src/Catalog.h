/**********************************************************************

  Sneedacity: A Digital Audio Editor

  Catalog.h

  moxniso

*******************************************************************//**

*//*******************************************************************/

#pragma once

class CatalogListener : public wxEvtHandler {
public:
	void CheckForSneed(wxKeyEvent& kev);
};
