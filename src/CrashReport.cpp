/**********************************************************************
 
 Sneedacity: A Digital Audio Editor
 
 CrashReport.cpp
 
 *//*******************************************************************/


#include "CrashReport.h"

#if defined(HAS_CRASH_REPORT)
#include <atomic>
#include <thread>

#include <wx/progdlg.h>

#if defined(__WXMSW__)
#include <wx/evtloop.h>
#endif

#include "wxFileNameWrapper.h"
#include "SneedacityLogger.h"
#include "AudioIOBase.h"
#include "FileNames.h"
#include "Internat.h"
#include "Languages.h"
#include "Project.h"
#include "ProjectFileIO.h"
#include "prefs/GUIPrefs.h"
#include "widgets/ErrorDialog.h"

namespace CrashReport {

void Generate(wxDebugReport::Context ctx)
{
   wxDebugReportCompress rpt;

   // Bug 1927: Seems there problems with wxStackWalker, so don't even include
   // the stack trace or memory dump. The former is pretty much useless in Release
   // builds anyway and none of use have the skill/time/desire to fiddle with the
   // latter.
   // rpt.AddAll(ctx);

   {
      // Provides a progress dialog with indeterminate mode
      wxGenericProgressDialog pd(XO("Sneedacity Support Data").Translation(),
                                 XO("This may take several seconds").Translation(),
                                 300000,     // range
                                 nullptr,    // parent
                                 wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_SMOOTH);

      std::atomic_bool done = {false};
      auto thread = std::thread([&]
      {
         wxFileNameWrapper fn{ FileNames::ConfigDir(), wxT("sneedacity.cfg") };
         rpt.AddFile(fn.GetFullPath(), _TS("Sneedacity Configuration"));
         rpt.AddFile(FileNames::PluginRegistry(), wxT("Plugin Registry"));
         rpt.AddFile(FileNames::PluginSettings(), wxT("Plugin Settings"));
   
         if (ctx == wxDebugReport::Context_Current)
         {
            auto saveLang = Languages::GetLangShort();
            GUIPrefs::SetLang( wxT("en") );
            auto cleanup = finally( [&]{ GUIPrefs::SetLang( saveLang ); } );
      
            auto gAudioIO = AudioIOBase::Get();
            rpt.AddText(wxT("audiodev.txt"), gAudioIO->GetDeviceInfo(), wxT("Audio Device Info"));
#ifdef EXPERIMENTAL_MIDI_OUT
            rpt.AddText(wxT("mididev.txt"), gAudioIO->GetMidiDeviceInfo(), wxT("MIDI Device Info"));
#endif
            auto project = GetActiveProject();
            auto &projectFileIO = ProjectFileIO::Get( *project );
            rpt.AddText(wxT("project.txt"), projectFileIO.GenerateDoc(), wxT("Active project doc"));
         }
   
         auto logger = SneedacityLogger::Get();
         if (logger)
         {
            rpt.AddText(wxT("log.txt"), logger->GetLog(), _TS("Sneedacity Log"));
         }
   
         done = true;
      });
   
      // Wait for information to be gathered
      while (!done)
      {
         wxMilliSleep(50);
         pd.Pulse();
      }
      thread.join();
   }

   bool ok = wxDebugReportPreviewStd().Show(rpt);
   
#if defined(__WXMSW__)
   wxEventLoop::SetCriticalWindow(NULL);
#endif
   
   if (ok && rpt.Process())
   {
      SneedacityTextEntryDialog dlg(nullptr,
         XO("Report generated to:"),
         XO("Sneedacity Support Data"),
         rpt.GetCompressedFileName(),
         wxOK | wxCENTER);
      dlg.SetName(dlg.GetTitle());
      dlg.ShowModal();
      
      wxLogMessage(wxT("Report generated to: %s"),
                   rpt.GetCompressedFileName());
      
      rpt.Reset();
   }
}

}
#endif
