/**********************************************************************

Sneedacity: A Digital Audio Editor

ProjectSettings.h

Paul Licameli split from SneedacityProject.h

**********************************************************************/

#ifndef __SNEEDACITY_PROJECT_SETTINGS__
#define __SNEEDACITY_PROJECT_SETTINGS__

#include <atomic>
#include <wx/event.h> // to declare custom event type

#include "ClientData.h" // to inherit
#include "Prefs.h" // to inherit

class SneedacityProject;

// Sent to the project when certain settings change
wxDECLARE_EXPORTED_EVENT(SNEEDACITY_DLL_API,
   EVT_PROJECT_SETTINGS_CHANGE, wxCommandEvent);

enum
{
   SNAP_OFF,
   SNAP_NEAREST,
   SNAP_PRIOR
};

namespace ToolCodes {
enum {
   selectTool,
   envelopeTool,
   drawTool,
   zoomTool,
   slideTool,
   multiTool,
   numTools,
   
   firstTool = selectTool,
   lastTool = multiTool,
};
}

///\brief Holds various per-project settings values, including the sample rate,
/// and sends events to the project when certain values change
class SNEEDACITY_DLL_API ProjectSettings final
   : public ClientData::Base
   , private PrefsListener
{
public:
   static ProjectSettings &Get( SneedacityProject &project );
   static const ProjectSettings &Get( const SneedacityProject &project );
   
   // Values retrievable from GetInt() of the event for settings change
   enum EventCode : int {
      ChangedSyncLock,
      ChangedProjectRate
   };

   explicit ProjectSettings( SneedacityProject &project );
   ProjectSettings( const ProjectSettings & ) PROHIBITED;
   ProjectSettings &operator=( const ProjectSettings & ) PROHIBITED;


   bool GetTracksFitVerticallyZoomed() const { return mTracksFitVerticallyZoomed; } //lda
   void SetTracksFitVerticallyZoomed(bool flag) { mTracksFitVerticallyZoomed = flag; } //lda

   bool GetShowId3Dialog() const { return mShowId3Dialog; } //lda
   void SetShowId3Dialog(bool flag) { mShowId3Dialog = flag; } //lda

   bool IsSyncLocked() const;
   void SetSyncLock(bool flag);
   
   // Rate

   void SetRate(double rate);
   double GetRate() const;

   // Snap To

   void SetSnapTo(int snap);
   int GetSnapTo() const;

   // Current tool

   void SetTool(int tool) { mCurrentTool = tool; }
   int GetTool() const { return mCurrentTool; }

   // Speed play
   double GetPlaySpeed() const {
      return mPlaySpeed.load( std::memory_order_relaxed ); }
   void SetPlaySpeed( double value ) {
      mPlaySpeed.store( value, std::memory_order_relaxed ); }

   // Selection Format
   void SetSelectionFormat(const NumericFormatSymbol & format);
   const NumericFormatSymbol & GetSelectionFormat() const;

   // AudioTime format
   void SetAudioTimeFormat(const NumericFormatSymbol & format);
   const NumericFormatSymbol & GetAudioTimeFormat() const;

   // Spectral Selection Formats
   void SetFrequencySelectionFormatName(const NumericFormatSymbol & format);
   const NumericFormatSymbol & GetFrequencySelectionFormatName() const;

   void SetBandwidthSelectionFormatName(const NumericFormatSymbol & format);
   const NumericFormatSymbol & GetBandwidthSelectionFormatName() const;

   bool IsSoloSimple() const { return mSoloPref == wxT("Simple"); }
   bool IsSoloNone() const { return mSoloPref == wxT("None"); }

   bool EmptyCanBeDirty() const { return mEmptyCanBeDirty; }

   bool GetShowSplashScreen() const { return mShowSplashScreen; }

private:
   void UpdatePrefs() override;

   SneedacityProject &mProject;

   NumericFormatSymbol mSelectionFormat;
   NumericFormatSymbol mFrequencySelectionFormatName;
   NumericFormatSymbol mBandwidthSelectionFormatName;
   NumericFormatSymbol mAudioTimeFormat;

   wxString mSoloPref;

   double mRate;

   // This is atomic because scrubber may read it in a separate thread from
   // the main
   std::atomic<double> mPlaySpeed{};

   int mSnapTo;

   int mCurrentTool;
   
   bool mTracksFitVerticallyZoomed{ false };  //lda
   bool mShowId3Dialog{ true }; //lda
   bool mIsSyncLocked{ false };
   bool mEmptyCanBeDirty;
   bool mShowSplashScreen;
};

#endif
