/**********************************************************************

  Sneedacity: A Digital Audio Editor

  DirectoriesPrefs.cpp

  Joshua Haberman
  James Crook


*******************************************************************//**

\class DirectoriesPrefs
\brief A PrefsPanel used to select directories.

*//*******************************************************************/


#include "DirectoriesPrefs.h"

#include <math.h>

#include <wx/defs.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/event.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/utils.h>

#include "../Prefs.h"
#include "../ShuttleGui.h"
#include "../TempDirectory.h"
#include "../widgets/SneedacityMessageBox.h"
#include "../widgets/ReadOnlyText.h"
#include "../widgets/wxTextCtrlWrapper.h"
#include "../FileNames.h"

using namespace FileNames;
using namespace TempDirectory;

class FilesystemValidator : public wxValidator
{
public:
   FilesystemValidator(const TranslatableString &message)
   :  wxValidator()
   {
      mMessage = message;
   }

   virtual wxObject* Clone() const wxOVERRIDE
   {
      return safenew FilesystemValidator(mMessage);
   }

   virtual bool Validate(wxWindow* WXUNUSED(parent)) wxOVERRIDE
   {
      wxTextCtrl* tc = wxDynamicCast(GetWindow(), wxTextCtrl);
      if (!tc) {
         return true;
      }

      if (FATFilesystemDenied(tc->GetValue(), mMessage)) {
         return false;
      }

      return true;
   }

   virtual bool TransferToWindow() wxOVERRIDE
   {
      return true;
   }

   virtual bool TransferFromWindow() wxOVERRIDE
   {
      return true;
   }

   void OnChar(wxKeyEvent &evt)
   {
      evt.Skip();

      wxTextCtrl* tc = wxDynamicCast(GetWindow(), wxTextCtrl);
      if (!tc) {
         return;
      }

      auto keycode = evt.GetUnicodeKey();
      if (keycode < WXK_SPACE || keycode == WXK_DELETE) {
         return;
      }

      wxString path = tc->GetValue();
      path.insert(tc->GetInsertionPoint(), keycode);

      if (FATFilesystemDenied(path, mMessage)) {
         evt.Skip(false);
         return;
      }
   }

   TranslatableString mMessage;

   wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(FilesystemValidator, wxValidator)
    EVT_CHAR(FilesystemValidator::OnChar)
wxEND_EVENT_TABLE()

enum
{
   TempTextID = 1000,
   TempButtonID,

   TextsStart = 1010,
   OpenTextID,
   SaveTextID,
   ImportTextID,
   ExportTextID,
   MacrosTextID,
   TextsEnd,

   ButtonsStart = 1020,
   OpenButtonID,
   SaveButtonID,
   ImportButtonID,
   ExportButtonID,
   MacrosButtonID,
   ButtonsEnd
};

BEGIN_EVENT_TABLE(DirectoriesPrefs, PrefsPanel)
   EVT_TEXT(TempTextID, DirectoriesPrefs::OnTempText)
   EVT_BUTTON(TempButtonID, DirectoriesPrefs::OnTempBrowse)
   EVT_COMMAND_RANGE(ButtonsStart, ButtonsEnd, wxEVT_BUTTON, DirectoriesPrefs::OnBrowse)
END_EVENT_TABLE()

DirectoriesPrefs::DirectoriesPrefs(wxWindow * parent, wxWindowID winid)
/* i18n-hint:  Directories, also called directories, in computer file systems */
:  PrefsPanel(parent, winid, XO("Directories")),
   mFreeSpace(NULL),
   mTempText(NULL)
{
   Populate();
}

DirectoriesPrefs::~DirectoriesPrefs()
{
}

ComponentInterfaceSymbol DirectoriesPrefs::GetSymbol()
{
   return DIRECTORIES_PREFS_PLUGIN_SYMBOL;
}

TranslatableString DirectoriesPrefs::GetDescription()
{
   return XO("Preferences for Directories");
}

ManualPageID DirectoriesPrefs::HelpPageName()
{
   return "Directories_Preferences";
}

/// Creates the dialog and its contents.
void DirectoriesPrefs::Populate()
{
   //------------------------- Main section --------------------
   // Now construct the GUI itself.
   // Use 'eIsCreatingFromPrefs' so that the GUI is
   // initialised with values from gPrefs.
   ShuttleGui S(this, eIsCreatingFromPrefs);
   PopulateOrExchange(S);
   // ----------------------- End of main section --------------

   wxCommandEvent e;
   OnTempText(e);
}

void DirectoriesPrefs::PopulateOrExchange(ShuttleGui &S)
{
   S.SetBorder(2);
   S.StartScroller();

   S.StartStatic(XO("Default directories"));
   {
      S.AddSpace(1);
      S.AddFixedText(XO("Leave a field empty to go to the last directory used for that operation.\n"
         "Fill in a field to always go to that directory for that operation."), false, 450);
      S.AddSpace(5);

      S.StartMultiColumn(3, wxEXPAND);
      {
         S.SetStretchyCol(1);

         S.Id(OpenTextID);
         mOpenText = S.TieTextBox(XXO("O&pen:"),
                                      {PreferenceKey(Operation::Open, PathType::User),
                                       wxT("")},
                                      30);
         S.Id(OpenButtonID).AddButton(XXO("&Browse..."));

         S.Id(SaveTextID);
         mSaveText = S.TieTextBox(XXO("S&ave:"),
                                      {PreferenceKey(Operation::Save, PathType::User),
                                       wxT("")},
                                      30);
         if( mSaveText )
            mSaveText->SetValidator(FilesystemValidator(XO("Projects cannot be saved to FAT drives.")));
         S.Id(SaveButtonID).AddButton(XXO("B&rowse..."));

         S.Id(ImportTextID);
         mImportText = S.TieTextBox(XXO("&Import:"),
                                    {PreferenceKey(Operation::Import, PathType::User),
                                     wxT("")},
                                    30);
         S.Id(ImportButtonID).AddButton(XXO("Br&owse..."));

         S.Id(ExportTextID);
         mExportText = S.TieTextBox(XXO("&Export:"),
                                    {PreferenceKey(Operation::Export, PathType::User),
                                     wxT("")},
                                    30);
         S.Id(ExportButtonID).AddButton(XXO("Bro&wse..."));

         S.Id(MacrosTextID);
         mMacrosText = S.TieTextBox(XXO("&Macro output:"),
                                    {PreferenceKey(Operation::MacrosOut, PathType::User),
                                     wxT("")},
                                    30);
         S.Id(MacrosButtonID).AddButton(XXO("Bro&wse..."));


      }
      S.EndMultiColumn();
   }
   S.EndStatic();

   S.StartStatic(XO("Temporary files directory"));
   {
      S.StartMultiColumn(3, wxEXPAND);
      {
         S.SetStretchyCol(1);

         S.Id(TempTextID);
         mTempText = S.TieTextBox(XXO("&Location:"),
                                  {PreferenceKey(Operation::Temp, PathType::_None),
                                   wxT("")},
                                  30);
         if( mTempText )
            mTempText->SetValidator(FilesystemValidator(XO("Temporary files directory cannot be on a FAT drive.")));
         S.Id(TempButtonID).AddButton(XXO("Brow&se..."));

         mFreeSpace = S
            .AddReadOnlyText(XXO("&Free Space:"), "");
      }
      S.EndMultiColumn();
   }
   S.EndStatic();

   S.EndScroller();
}

void DirectoriesPrefs::OnTempBrowse(wxCommandEvent &evt)
{
   wxString oldTemp = gPrefs->Read(PreferenceKey(Operation::Open, PathType::_None),
                                   DefaultTempDir());

   // Because we went through InitTemp() during initialisation,
   // the old temp directory name in prefs should already be OK.  Just in case there is 
   // some way we hadn't thought of for it to be not OK, 
   // we avoid prompting with it in that case and use the suggested default instead.
   if (!IsTempDirectoryNameOK(oldTemp))
   {
      oldTemp = DefaultTempDir();
   }

   wxDirDialogWrapper dlog(this,
                           XO("Choose a location to place the temporary directory"),
                           oldTemp);
   int retval = dlog.ShowModal();
   if (retval != wxID_CANCEL && !dlog.GetPath().empty())
   {
      wxFileName tmpDirPath;
      tmpDirPath.AssignDir(dlog.GetPath());

      if (FATFilesystemDenied(tmpDirPath.GetFullPath(),
          XO("Temporary files directory cannot be on a FAT drive."))) {
         return;
      }

      if (!FileNames::WritableLocationCheck(dlog.GetPath()))
      {
         return;
      }

      // Append an "sneedacity_temp" directory to this path if necessary (the
      // default, the existing pref (as stored in the control), and any path
      // ending in a directory with the same name as what we'd add should be OK
      // already)
      wxString newDirName;
#if defined(__WXMAC__)
      newDirName = wxT("SessionData");
#elif defined(__WXMSW__) 
      // Clearing Bug 1271 residual issue.  Let's NOT have temp in the name.
      newDirName = wxT("SessionData");
#else
      newDirName = wxT(".sneedacity_temp");
#endif
      auto dirsInPath = tmpDirPath.GetDirs();

      // If the default temp dir or user's pref dir don't end in '/' they cause
      // wxFileName's == operator to construct a wxFileName representing a file
      // (that doesn't exist) -- hence the constructor calls
      if (tmpDirPath != wxFileName(DefaultTempDir(), wxT("")) &&
            tmpDirPath != wxFileName(mTempText->GetValue(), wxT("")) &&
            (dirsInPath.size() == 0 ||
             dirsInPath[dirsInPath.size()-1] != newDirName))
      {
         tmpDirPath.AppendDir(newDirName);
      }

      mTempText->SetValue(tmpDirPath.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR));
      OnTempText(evt);
   }
}

void DirectoriesPrefs::OnTempText(wxCommandEvent & WXUNUSED(evt))
{
   TranslatableString label;

   if (mTempText && mFreeSpace)
   {
      FilePath path = mTempText->GetValue();

      wxLongLong space;
      wxGetDiskSpace(path, NULL, &space);

      label = wxDirExists(path)
         ? Internat::FormatSize(space)
         : XO("unavailable - above location doesn't exist");

      mFreeSpace->SetValue(label.Translation());
   }
}

void DirectoriesPrefs::OnBrowse(wxCommandEvent &evt)
{
   long id = evt.GetId() - ButtonsStart;
   wxTextCtrl *tc = (wxTextCtrl *) FindWindow(id + TextsStart);

   wxString location = tc->GetValue();

   wxDirDialogWrapper dlog(this,
                           XO("Choose a location"),
                           location);
   int retval = dlog.ShowModal();

   if (retval == wxID_CANCEL)
   {
      return;
   }

   if (evt.GetId() == SaveButtonID)
   {
      if (FATFilesystemDenied(dlog.GetPath(),
                              XO("Projects cannot be saved to FAT drives.")))
      {
         return;
      }
   }

   if (!FileNames::WritableLocationCheck(dlog.GetPath()))
   {
      return;
   }

   tc->SetValue(dlog.GetPath());
}

bool DirectoriesPrefs::Validate()
{
   wxFileName Temp;
   Temp.SetPath(mTempText->GetValue());

   wxString path{Temp.GetPath()};
   if( !IsTempDirectoryNameOK( path ) ) {
      SneedacityMessageBox(
         XO("Directory %s is not suitable (at risk of being cleaned out)")
            .Format( path ),
         XO("Error"),
         wxOK | wxICON_ERROR);
      return false;
   }

   if (!Temp.DirExists()) {
      int ans = SneedacityMessageBox(
         XO("Directory %s does not exist. Create it?")
            .Format( path ),
         XO("New Temporary Directory"),
         wxYES_NO | wxCENTRE | wxICON_EXCLAMATION);

      if (ans != wxYES) {
         return false;
      }

      if (!Temp.Mkdir(0755, wxPATH_MKDIR_FULL)) {
         /* wxWidgets throws up a decent looking dialog */
         return false;
      }
   }
   else {
      /* If the directory already exists, make sure it is writable */
      wxLogNull logNo;
      Temp.AppendDir(wxT("canicreate"));
      path =  Temp.GetPath();
      if (!Temp.Mkdir(0755)) {
         SneedacityMessageBox(
            XO("Directory %s is not writable")
               .Format( path ),
            XO("Error"),
            wxOK | wxICON_ERROR);
         return false;
      }
      Temp.Rmdir();
      Temp.RemoveLastDir();
   }

   wxFileName oldDir;
   oldDir.SetPath(TempDir());
   if (Temp != oldDir) {
      SneedacityMessageBox(
         XO(
"Changes to temporary directory will not take effect until Sneedacity is restarted"),
         XO("Temp Directory Update"),
         wxOK | wxCENTRE | wxICON_INFORMATION);
   }

   return true;
}

bool DirectoriesPrefs::Commit()
{
   ShuttleGui S(this, eIsSavingToPrefs);
   PopulateOrExchange(S);

   return true;
}

PrefsPanel::Factory
DirectoriesPrefsFactory()
{
   return [](wxWindow *parent, wxWindowID winid, SneedacityProject *)
   {
      wxASSERT(parent); // to justify safenew
      return safenew DirectoriesPrefs(parent, winid);
   };
}

namespace
{
   PrefsPanel::Registration sAttachment
   {
      "Directories",
      DirectoriesPrefsFactory()
   };
};

