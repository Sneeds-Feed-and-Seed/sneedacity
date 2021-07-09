/**********************************************************************

Sneedacity: A Digital Audio Editor

ProjectSettings.cpp

Paul Licameli split from SneedacityProject.cpp

**********************************************************************/

#include "ProjectSettings.h"



#include "AudioIOBase.h"
#include "Project.h"
#include "prefs/QualitySettings.h"
#include "widgets/NumericTextCtrl.h"
#include "prefs/TracksBehaviorsPrefs.h"

wxDEFINE_EVENT(EVT_PROJECT_SETTINGS_CHANGE, wxCommandEvent);

namespace {
   void Notify( SneedacityProject &project, ProjectSettings::EventCode code )
   {
      wxCommandEvent e{ EVT_PROJECT_SETTINGS_CHANGE };
      e.SetInt( static_cast<int>( code ) );
      project.ProcessEvent( e );
   }
}

static const SneedacityProject::AttachedObjects::RegisteredFactory
sProjectSettingsKey{
  []( SneedacityProject &project ){
     auto result = std::make_shared< ProjectSettings >( project );
     return result;
   }
};

ProjectSettings &ProjectSettings::Get( SneedacityProject &project )
{
   return project.AttachedObjects::Get< ProjectSettings >(
      sProjectSettingsKey );
}

const ProjectSettings &ProjectSettings::Get( const SneedacityProject &project )
{
   return Get( const_cast< SneedacityProject & >( project ) );
}

ProjectSettings::ProjectSettings(SneedacityProject &project)
   : mProject{ project }
   , mSelectionFormat{ NumericTextCtrl::LookupFormat(
      NumericConverter::TIME,
      gPrefs->Read(wxT("/SelectionFormat"), wxT("")))
}
, mAudioTimeFormat{ NumericTextCtrl::LookupFormat(
   NumericConverter::TIME,
   gPrefs->Read(wxT("/AudioTimeFormat"), wxT("hh:mm:ss")))
}
, mFrequencySelectionFormatName{ NumericTextCtrl::LookupFormat(
   NumericConverter::FREQUENCY,
   gPrefs->Read(wxT("/FrequencySelectionFormatName"), wxT("")) )
}
, mBandwidthSelectionFormatName{ NumericTextCtrl::LookupFormat(
   NumericConverter::BANDWIDTH,
   gPrefs->Read(wxT("/BandwidthSelectionFormatName"), wxT("")) )
}
, mSnapTo( gPrefs->Read(wxT("/SnapTo"), SNAP_OFF) )
{
   int intRate = 0;
   bool wasDefined = QualitySettings::DefaultSampleRate.Read( &intRate );
   mRate = intRate;
   if ( !wasDefined ) {
      // The default given above can vary with host/devices. So unless there is
      // an entry for the default sample rate in sneedacity.cfg, Sneedacity can open
      // with a rate which is different from the rate with which it closed.
      // See bug 1879.
      QualitySettings::DefaultSampleRate.Write( mRate );
      gPrefs->Flush();
   }
   gPrefs->Read(wxT("/GUI/SyncLockTracks"), &mIsSyncLocked, false);

   bool multiToolActive = false;
   gPrefs->Read(wxT("/GUI/ToolBars/Tools/MultiToolActive"), &multiToolActive);

   if (multiToolActive)
      mCurrentTool = ToolCodes::multiTool;
   else
      mCurrentTool = ToolCodes::selectTool;

   UpdatePrefs();
}

void ProjectSettings::UpdatePrefs()
{
   gPrefs->Read(wxT("/AudioFiles/ShowId3Dialog"), &mShowId3Dialog, true);
   gPrefs->Read(wxT("/GUI/EmptyCanBeDirty"), &mEmptyCanBeDirty, true);
   gPrefs->Read(wxT("/GUI/ShowSplashScreen"), &mShowSplashScreen, true);
   mSoloPref = TracksBehaviorsSolo.Read();
   // Update the old default to the NEW default.
   if (mSoloPref == wxT("Standard"))
      mSoloPref = wxT("Simple");
   gPrefs->Read(wxT("/GUI/TracksFitVerticallyZoomed"),
      &mTracksFitVerticallyZoomed, false);
   //   gPrefs->Read(wxT("/GUI/UpdateSpectrogram"),
   //     &mViewInfo.bUpdateSpectrogram, true);

   // This code to change an empty projects rate is currently disabled, after
   // discussion.  The rule 'Default sample rate' only affects newly created
   // projects was felt to be simpler and better.
#if 0
   // The DefaultProjectSample rate is the rate for new projects.
   // Do not change this project's rate, unless there are no tracks.
   if( TrackList::Get( *this ).size() == 0){
      mRate = QualityDefaultSampleRate.Read();
      // If necessary, we change this rate in the selection toolbar too.
      auto bar = SelectionBar::Get( *this );
      bar.SetRate( mRate );
   }
#endif
}

const NumericFormatSymbol &
ProjectSettings::GetFrequencySelectionFormatName() const
{
   return mFrequencySelectionFormatName;
}

void ProjectSettings::SetFrequencySelectionFormatName(
   const NumericFormatSymbol & formatName)
{
   mFrequencySelectionFormatName = formatName;
}

const NumericFormatSymbol &
ProjectSettings::GetBandwidthSelectionFormatName() const
{
   return mBandwidthSelectionFormatName;
}

void ProjectSettings::SetBandwidthSelectionFormatName(
   const NumericFormatSymbol & formatName)
{
   mBandwidthSelectionFormatName = formatName;
}

void ProjectSettings::SetSelectionFormat(const NumericFormatSymbol & format)
{
   mSelectionFormat = format;
}

const NumericFormatSymbol & ProjectSettings::GetSelectionFormat() const
{
   return mSelectionFormat;
}

void ProjectSettings::SetAudioTimeFormat(const NumericFormatSymbol & format)
{
   mAudioTimeFormat = format;
}

const NumericFormatSymbol & ProjectSettings::GetAudioTimeFormat() const
{
   return mAudioTimeFormat;
}

double ProjectSettings::GetRate() const
{
   return mRate;
}

void ProjectSettings::SetRate(double rate)
{
   auto &project = mProject;
   if (rate != mRate) {
      mRate = rate;
      Notify( project, ChangedProjectRate );
   }
}

void ProjectSettings::SetSnapTo(int snap)
{
   mSnapTo = snap;
}
   
int ProjectSettings::GetSnapTo() const
{
   return mSnapTo;
}

bool ProjectSettings::IsSyncLocked() const
{
#ifdef EXPERIMENTAL_SYNC_LOCK
   return mIsSyncLocked;
#else
   return false;
#endif
}

void ProjectSettings::SetSyncLock(bool flag)
{
   auto &project = mProject;
   if (flag != mIsSyncLocked) {
      mIsSyncLocked = flag;
      Notify( project, ChangedSyncLock );
   }
}

