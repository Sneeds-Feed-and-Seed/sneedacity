/**********************************************************************

  Sneedacity: A Digital Audio Editor

  AboutDialog.h

  Dominic Mazzoni

**********************************************************************/

#ifndef __SNEEDACITY_ABOUT_DLG__
#define __SNEEDACITY_ABOUT_DLG__

#include <vector>
#include "widgets/wxPanelWrapper.h" // to inherit

class wxStaticBitmap;
class wxTextOutputStream;

class ShuttleGui;

struct AboutDialogCreditItem {
   AboutDialogCreditItem( TranslatableString str, int r )
      : description{ std::move( str ) }, role{ r }
   {}
   TranslatableString description;
   int role;  
};

using AboutDialogCreditItemsList = std::vector<AboutDialogCreditItem>;

class SNEEDACITY_DLL_API AboutDialog final : public wxDialogWrapper {
   DECLARE_DYNAMIC_CLASS(AboutDialog)

 public:
   AboutDialog(wxWindow * parent);
   virtual ~ AboutDialog();

   static AboutDialog *ActiveIntance();

   void OnOK(wxCommandEvent & event);

   wxStaticBitmap *icon;

   DECLARE_EVENT_TABLE()

 private:
   enum Role {
      roleTeamMember,
      roleEmeritusTeam,
      roleDeceased,
      roleContributor,
      roleGraphics,
      roleLibrary,
      roleThanks,
      roleSneed
   };

   AboutDialogCreditItemsList creditItems;
   void PopulateSneedacityPage( ShuttleGui & S );
   void PopulateLicensePage( ShuttleGui & S );
   void PopulateInformationPage (ShuttleGui & S );

   void CreateCreditsList();
   void AddCredit( const wxString &name, Role role );
   void AddCredit( const wxString &name, TranslatableString format, Role role );
   wxString GetCreditsByRole(AboutDialog::Role role);

   void AddBuildinfoRow( wxTextOutputStream *str, const wxChar * libname,
      const TranslatableString &libdesc, const TranslatableString &status);
   void AddBuildinfoRow( wxTextOutputStream *str,
      const TranslatableString &description, const wxChar *spec);
};

#endif
