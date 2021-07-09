/**********************************************************************

  Sneedacity: A Digital Audio Editor

  TimeToolBar.h

  Jonatã Bolzan Loss

**********************************************************************/

#ifndef __SNEEDACITY_TIME_TOOLBAR__
#define __SNEEDACITY_TIME_TOOLBAR__

#include <wx/defs.h>

#include "ToolBar.h"
#include "../widgets/NumericTextCtrl.h"

class NumericTextCtrl;
class TimeToolBarListener;

class TimeToolBar final : public ToolBar
{
public:
   TimeToolBar(SneedacityProject &project);
   virtual ~TimeToolBar();
   
   static TimeToolBar &Get(SneedacityProject &project);
   static const TimeToolBar &Get(const SneedacityProject &project);
   
   void Populate() override;
   void Repaint(wxDC * WXUNUSED(dc)) override {};
   void EnableDisableButtons() override {};
   void UpdatePrefs() override;
   void RegenerateTooltips() override {};
   int GetInitialWidth() override {return 250;} 
   int GetMinToolbarWidth() override {return 50;}
   void SetToDefaultSize() override;
   wxSize GetDockedSize() override;
   void SetDocked(ToolDock *dock, bool pushed) override;
   void SetListener(TimeToolBarListener *l);
   void SetAudioTimeFormat(const NumericFormatSymbol & format);
   void ResizingDone() override;

private:
   void SetResizingLimits();
   wxSize ComputeSizing(int digitH);

   void OnSettingsChanged(wxCommandEvent &evt);
   void OnUpdate(wxCommandEvent &evt);
   void OnSize(wxSizeEvent &evt);
   void OnIdle(wxIdleEvent &evt);

   TimeToolBarListener *mListener;
   NumericTextCtrl *mAudioTime;
   float mDigitRatio;
   bool mSettingInitialSize;

   static const int minDigitH = 17;
   static const int maxDigitH = 100;

public:
   
   DECLARE_CLASS(TimeToolBar)
   DECLARE_EVENT_TABLE()
};

inline wxSize TimeToolBar::ComputeSizing(int digitH)
{
   return mAudioTime->ComputeSizing(false, digitH * mDigitRatio, digitH);
}

#endif
