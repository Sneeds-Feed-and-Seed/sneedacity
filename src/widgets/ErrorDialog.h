/**********************************************************************

  Sneedacity: A Digital Audio Editor

  ErrorDialog.h

  Jimmy Johnson
  James Crook

**********************************************************************/

#ifndef __SNEEDACITY_ERRORDIALOG__
#define __SNEEDACITY_ERRORDIALOG__



#include <string>
#include <wx/defs.h>
#include <wx/msgdlg.h> // to inherit
#include "wxPanelWrapper.h" // to inherit

class SneedacityProject;
class wxCollapsiblePaneEvent;

class ErrorDialog /* not final */ : public wxDialogWrapper
{
public:
   // constructors and destructors
   ErrorDialog(wxWindow *parent,
      const TranslatableString & dlogTitle,
      const TranslatableString & message,
      const ManualPageID & helpPage,
      const std::wstring & log,
      const bool Close = true, const bool modal = true);

   virtual ~ErrorDialog(){}

private:
   ManualPageID dhelpPage;
   bool dClose;
   bool dModal;

   void OnPane( wxCollapsiblePaneEvent &event );
   void OnOk( wxCommandEvent &event );
   void OnHelp( wxCommandEvent &event );
   DECLARE_EVENT_TABLE()
};

/// Displays an error dialog with a button that offers help
SNEEDACITY_DLL_API
void ShowErrorDialog(wxWindow *parent,
                     const TranslatableString &dlogTitle,
                     const TranslatableString &message,
                     const ManualPageID &helpPage,
                     bool Close = true,
                     const std::wstring &log = {});

/// Displays an error dialog, possibly allowing to send error report.
SNEEDACITY_DLL_API
void ShowExceptionDialog(
   wxWindow* parent, const TranslatableString& dlogTitle,
   const TranslatableString& message, const wxString& helpPage,
   bool Close = true, const wxString& log = {});

/// Displays a modeless error dialog with a button that offers help
void ShowModelessErrorDialog(wxWindow *parent,
                     const TranslatableString &dlogTitle,
                     const TranslatableString &message,
                     const ManualPageID &helpPage,
                     bool Close = true,
                     const std::wstring &log = {});

#include <wx/textdlg.h> // to inherit

/**************************************************************************//**
\class SneedacityTextEntryDialog
\brief Wrap wxTextEntryDialog so that caption IS translatable.
********************************************************************************/
class SNEEDACITY_DLL_API SneedacityTextEntryDialog
   : public wxTabTraversalWrapper< wxTextEntryDialog >
{
public:
    SneedacityTextEntryDialog(
         wxWindow *parent,
         const TranslatableString& message,
         const TranslatableString& caption, // don't use = wxGetTextFromUserPromptStr,
         const wxString& value = {},
         long style = wxTextEntryDialogStyle,
         const wxPoint& pos = wxDefaultPosition)
   : wxTabTraversalWrapper< wxTextEntryDialog>(
      parent,
      message.Translation(), caption.Translation(), value, style, pos )
   {}
   
   void SetInsertionPointEnd();
   bool Show(bool show = true) override;

private:
   bool mSetInsertionPointEnd{};
};

#endif // __SNEEDACITY_ERRORDIALOG__
