/**********************************************************************

  Sneedacity: A Digital Audio Editor

  PluginRegistrationDialog.h

  Paul Licameli split from PluginManager.cpp

**********************************************************************/
#ifndef __SNEEDACITY_PLUGIN_REGISTRATION_DIALOG__
#define __SNEEDACITY_PLUGIN_REGISTRATION_DIALOG__

#include "widgets/wxPanelWrapper.h" // to inherit
#include <vector>
#include <unordered_map> // member

class CheckListAx;
enum EffectType : int;
class PluginDescriptor;
class ShuttleGui;
class wxListEvent;
class wxListCtrl;

class PluginRegistrationDialog final : public wxDialogWrapper
{
public:
   // constructors and destructors
   PluginRegistrationDialog(wxWindow *parent, EffectType type);

private:
   struct ItemData
   {
      std::vector<PluginDescriptor*> plugs;
      wxString name;
      PluginPath path;
      int state;
      bool valid;
      int nameWidth;
      int pathWidth;
      int stateWidth;
   };

   using ItemDataMap = std::unordered_map<PluginPath, ItemData>;

   void Populate();
   void PopulateOrExchange(ShuttleGui & S);
   void RegenerateEffectsList(int iShowWhat);
   void SetState(int i, bool toggle, bool state = true);

   static int wxCALLBACK SortCompare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
   int SortCompare(ItemData *item1, ItemData *item2);

   void OnChangedVisibility(wxCommandEvent & evt);
   void OnSort(wxListEvent & evt);
   void DoSort( int col );
   void OnListChar(wxKeyEvent & evt);
   void OnOK(wxCommandEvent & evt);
   void OnCancel(wxCommandEvent & evt);
   void OnSelectAll(wxCommandEvent & evt);
   void OnClearAll(wxCommandEvent & evt);
   void OnEnable(wxCommandEvent & evt);
   void OnDisable(wxCommandEvent & evt);

private:
   EffectType mType;
   int mFilter;

   wxArrayString mStates;
   ItemDataMap mItems;

   int mSortColumn;
   int mSortDirection;

   PluginPath mLongestPath;

   wxListCtrl *mEffects;
#if wxUSE_ACCESSIBILITY
   CheckListAx *mAx;
#endif

   DECLARE_EVENT_TABLE()
};


#endif
