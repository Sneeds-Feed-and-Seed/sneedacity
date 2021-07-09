//
//  wxPanelWrapper.h
//  Sneedacity
//
//  Created by Paul Licameli on 6/25/16.
//
//

#ifndef __SNEEDACITY_WXPANEL_WRAPPER__
#define __SNEEDACITY_WXPANEL_WRAPPER__

#include <memory>
#include <wx/panel.h> // to inherit
#include <wx/dialog.h> // to inherit

#include "Internat.h"

#include "Identifier.h"

SNEEDACITY_DLL_API void wxTabTraversalWrapperCharHook(wxKeyEvent &event);

template <typename Base>
class SNEEDACITY_DLL_API wxTabTraversalWrapper : public Base
{
public:
   template <typename... Args>
   wxTabTraversalWrapper(Args&&... args)
   : Base( std::forward<Args>(args)... )
   {
      this->Bind(wxEVT_CHAR_HOOK, wxTabTraversalWrapperCharHook);
   }

   wxTabTraversalWrapper(const wxTabTraversalWrapper&) = delete;
   wxTabTraversalWrapper& operator=(const wxTabTraversalWrapper&) = delete;
   wxTabTraversalWrapper(wxTabTraversalWrapper&&) = delete;
   wxTabTraversalWrapper& operator=(wxTabTraversalWrapper&&) = delete;

};

class SNEEDACITY_DLL_API wxPanelWrapper : public wxTabTraversalWrapper<wxPanel>
{
public:
   // Constructors
   wxPanelWrapper() {}

   wxPanelWrapper(
         wxWindow *parent,
         wxWindowID winid = wxID_ANY,
         const wxPoint& pos = wxDefaultPosition,
         const wxSize& size = wxDefaultSize,
         long style = wxTAB_TRAVERSAL | wxNO_BORDER,
         // Important:  default window name localizes!
         const TranslatableString& name = XO("Panel"))
   : wxTabTraversalWrapper<wxPanel> (
      parent, winid, pos, size, style, name.Translation() )
   {}

    // Pseudo ctor
    bool Create(
         wxWindow *parent,
         wxWindowID winid = wxID_ANY,
         const wxPoint& pos = wxDefaultPosition,
         const wxSize& size = wxDefaultSize,
         long style = wxTAB_TRAVERSAL | wxNO_BORDER,
         // Important:  default window name localizes!
         const TranslatableString& name = XO("Panel"))
   {
      return wxTabTraversalWrapper<wxPanel>::Create(
         parent, winid, pos, size, style, name.Translation()
      );
   }
   // overload and hide the inherited functions that take naked wxString:
   void SetLabel(const TranslatableString & label);
   void SetName(const TranslatableString & name);
   void SetToolTip(const TranslatableString &toolTip);
   // Set the name to equal the label:
   void SetName();
};

class SNEEDACITY_DLL_API wxDialogWrapper : public wxTabTraversalWrapper<wxDialog>
{
public:
   // Constructors
   wxDialogWrapper() {}

   // Constructor with no modal flag - the new convention.
   wxDialogWrapper(
      wxWindow *parent, wxWindowID id,
      const TranslatableString& title,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      long style = wxDEFAULT_DIALOG_STYLE,
      // Important:  default window name localizes!
      const TranslatableString& name = XO("Dialog"))
   : wxTabTraversalWrapper<wxDialog>(
      parent, id, title.Translation(), pos, size, style, name.Translation() )
   {}

   // Pseudo ctor
   bool Create(
      wxWindow *parent, wxWindowID id,
      const TranslatableString& title,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      long style = wxDEFAULT_DIALOG_STYLE,
      // Important:  default window name localizes!
      const TranslatableString& name = XO("Dialog"))
   {
      return wxTabTraversalWrapper<wxDialog>::Create(
         parent, id, title.Translation(), pos, size, style, name.Translation()
      );
   }

   // overload and hide the inherited functions that take naked wxString:
   void SetTitle(const TranslatableString & title);
   void SetLabel(const TranslatableString & title);
   void SetName(const TranslatableString & title);
   // Set the name to equal the title:
   void SetName();
};

#include <wx/dirdlg.h> // to inherit

class SNEEDACITY_DLL_API wxDirDialogWrapper
   : public wxTabTraversalWrapper<wxDirDialog>
{
public:
   // Constructor with no modal flag - the new convention.
   wxDirDialogWrapper(
      wxWindow *parent,
      const TranslatableString& message = XO("Select a directory"),
      const wxString& defaultPath = {},
      long style = wxDD_DEFAULT_STYLE,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      // Important:  default window name localizes!
      const TranslatableString& name = XO("Directory Dialog"))
   : wxTabTraversalWrapper<wxDirDialog>(
      parent, message.Translation(), defaultPath, style, pos, size,
      name.Translation() )
   {}

   // Pseudo ctor
   void Create(
      wxWindow *parent,
      const TranslatableString& message = XO("Select a directory"),
      const wxString& defaultPath = {},
      long style = wxDD_DEFAULT_STYLE,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      // Important:  default window name localizes!
      const TranslatableString& name = XO("Directory Dialog"))
   {
      wxTabTraversalWrapper<wxDirDialog>::Create(
         parent, message.Translation(), defaultPath, style, pos, size,
         name.Translation() );
   }
};

#include "FileDialog/FileDialog.h"
#include "../FileNames.h" // for FileTypes

class SNEEDACITY_DLL_API FileDialogWrapper
   : public wxTabTraversalWrapper<FileDialog>
{
public:
   FileDialogWrapper() {}

   // Constructor with no modal flag - the new convention.
   FileDialogWrapper(
      wxWindow *parent,
      const TranslatableString& message,
      const FilePath& defaultDir,
      const FilePath& defaultFile,
      const FileNames::FileTypes& fileTypes,
      long style = wxFD_DEFAULT_STYLE,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& sz = wxDefaultSize,
      // Important:  default window name localizes!
      const TranslatableString& name = XO("File Dialog"))
   : wxTabTraversalWrapper<FileDialog>(
      parent, message.Translation(), defaultDir, defaultFile,
      FileNames::FormatWildcard( fileTypes ),
      style, pos, sz, name.Translation() )
   {}

   // Pseudo ctor
   void Create(
      wxWindow *parent,
      const TranslatableString& message,
      const FilePath& defaultDir,
      const FilePath& defaultFile,
      const FileNames::FileTypes& fileTypes,
      long style = wxFD_DEFAULT_STYLE,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& sz = wxDefaultSize,
      // Important:  default window name localizes!
      const TranslatableString& name = XO("File Dialog"))
   {
      wxTabTraversalWrapper<FileDialog>::Create(
         parent, message.Translation(), defaultDir, defaultFile,
         FileNames::FormatWildcard( fileTypes ),
         style, pos, sz, name.Translation()
      );
   }
};

#include <wx/msgdlg.h>

/**************************************************************************//**

\brief Wrap wxMessageDialog so that caption IS translatable.
********************************************************************************/
class SneedacityMessageDialog : public wxTabTraversalWrapper< wxMessageDialog >
{
public:
    SneedacityMessageDialog(
         wxWindow *parent,
         const TranslatableString &message,
         const TranslatableString &caption, // don't use = wxMessageBoxCaptionStr,
         long style = wxOK|wxCENTRE,
         const wxPoint& pos = wxDefaultPosition)
   : wxTabTraversalWrapper< wxMessageDialog>
      ( parent, message.Translation(), caption.Translation(), style, pos )
   {}
};

#endif
